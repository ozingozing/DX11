////////////////////////////////////////////////////////////////////////////////
// 파일명: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "textureclass.h"

// 클래스 생성자에서 세 개의 포인터를 null로 초기화합니다.
TextureClass::TextureClass()
{
	m_targaData = 0;
	m_texture = 0;
	m_textureView = 0;
}


TextureClass::TextureClass(const TextureClass& other)
{
}


TextureClass::~TextureClass()
{
}
// Initialize 함수는 Direct3D 장치와 타가(Targa) 이미지 파일명을 입력으로 받습니다.
// 먼저 타가 데이터를 배열에 로드합니다. 그런 다음 텍스처를 생성하고 올바른 형식으로 타가 데이터를 로드합니다.
// (타가 이미지는 기본적으로 뒤집혀 있으므로 뒤집어야 합니다.)
// 텍스처가 로드되면 셰이더가 그리기에 사용할 수 있도록 텍스처에 대한 리소스 뷰를 생성합니다.
bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;
	int height, width;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	// 먼저 TextureClass::LoadTarga32Bit 함수를 호출하여 타가 파일을 m_targaData 배열에 로드합니다.
		// 타가 이미지 데이터를 메모리로 로드합니다.
	//result = LoadTarga32Bit(hwnd,filename);
	result = LoadImageFile(filename);
	if (!result)
	{
		return false;
	}
	// 다음으로, 타가 데이터를 로드할 DirectX 텍스처의 설명을 설정해야 합니다.
	// 타가 이미지 데이터의 높이와 너비를 사용하고, 형식을 32비트 RGBA 텍스처로 설정합니다.
	// SampleDesc는 기본값으로 설정합니다. 그런 다음 Usage를 D3D11_USAGE_DEFAULT로 설정하는데,
	// 이것은 더 좋은 성능의 메모리이며 아래에서 더 자세히 설명하겠습니다.
	// 마지막으로 MipLevels, BindFlags, MiscFlags를 밉맵 텍스처에 필요한 설정으로 지정합니다.
	// 설명이 완료되면 CreateTexture2D를 호출하여 빈 텍스처를 생성합니다. 다음 단계는 이 빈 텍스처에 타가 데이터를 복사하는 것입니다.
		// 텍스처의 설명을 설정합니다.
	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// 빈 텍스처를 생성합니다.
	hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	// 타가 이미지 데이터의 로우 피치(row pitch)를 설정합니다.
	rowPitch = (m_width * 4) * sizeof(unsigned char);
	// 여기서는 UpdateSubresource를 사용하여 실제로 타가 데이터 배열을 DirectX 텍스처로 복사합니다.
	// 이전 튜토리얼에서 행렬을 상수 버퍼에 복사하기 위해 Map과 Unmap을 사용했음을 기억할 것입니다.
	// 텍스처 데이터에도 동일한 작업을 할 수 있습니다. 실제로 Map과 Unmap을 사용하는 것이 UpdateSubresource보다
	// 훨씬 빠르지만, 두 로딩 방법은 각각 특정 목적이 있으며 성능을 위해 올바른 것을 선택해야 합니다.
	// 매 프레임마다 또는 매우 자주 다시 로드되는 데이터에는 Map과 Unmap을 사용하는 것이 좋습니다.
	// 반면, 한 번 로드되거나 로딩 시퀀스 중에 드물게 로드되는 데이터에는 UpdateSubresource를 사용해야 합니다.
	// 그 이유는 UpdateSubresource는 데이터를 곧 제거하거나 다시 로드하지 않을 것임을 알고 있기 때문에
	// 캐시 보존 우선순위가 높은 고속 메모리에 데이터를 배치합니다.
	// UpdateSubresource를 사용하여 로드할 때 D3D11_USAGE_DEFAULT를 사용함으로써 DirectX에 이를 알려줍니다.
	// Map과 Unmap은 DirectX가 데이터가 곧 덮어쓰여질 것으로 예상하므로 캐시되지 않는 메모리 위치에 데이터를 배치합니다.
	// 이것이 우리가 D3D11_USAGE_DYNAMIC을 사용하여 이 유형의 데이터가 임시적임을 DirectX에 알리는 이유입니다.
		// 타가 이미지 데이터를 텍스처로 복사합니다.
	deviceContext->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);
	// 텍스처가 로드된 후, 셰이더에서 텍스처를 설정할 포인터를 제공하는 셰이더 리소스 뷰를 생성합니다.
	// 설명에서는 두 개의 중요한 밉맵 변수를 설정하여 어떤 거리에서도 고품질 텍스처 렌더링을 위해
	// 전체 밉맵 레벨 범위를 제공합니다. 셰이더 리소스 뷰가 생성되면 GenerateMips를 호출하여 밉맵을 생성합니다.
	// 더 나은 품질을 원한다면 밉맵 레벨을 수동으로 로드할 수도 있습니다.
		// 셰이더 리소스 뷰 설명을 설정합니다.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// 텍스처에 대한 셰이더 리소스 뷰를 생성합니다.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// 이 텍스처에 대한 밉맵을 생성합니다.
	deviceContext->GenerateMips(m_textureView);

	// 이미지 데이터가 텍스처에 로드되었으므로 타가 이미지 데이터를 해제합니다.
	delete[] m_targaData;
	m_targaData = 0;

	return true;
}
// Shutdown 함수는 텍스처 데이터를 해제하고 세 개의 포인터를 null로 설정합니다.
void TextureClass::Shutdown()
{
	// 텍스처 뷰 리소스를 해제합니다.
	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = 0;
	}

	// 텍스처를 해제합니다.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	// 타가 데이터를 해제합니다.
	if (m_targaData)
	{
		delete[] m_targaData;
		m_targaData = 0;
	}

	return;
}
// GetTexture는 렌더링에 필요한 셰이더에 텍스처 뷰에 대한 쉬운 접근을 제공하는 헬퍼 함수입니다.
ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_textureView;
}

// TextureClass 클래스의 멤버 함수로 추가
bool TextureClass::LoadImageFile(const char* filename)
{
	int channels;
	// stbi_load 함수로 이미지를 RGBA 32비트 포맷으로 로드
	m_targaData = stbi_load(filename, &m_width, &m_height, &channels, STBI_rgb_alpha);

	// 로드 실패 시
	if (!m_targaData)
	{
		return false;
	}

	return true;
}

// 이 함수는 타가 이미지 로딩 함수입니다. 타가 이미지는 거꾸로 저장되므로 사용하기 전에 뒤집어야 함을 다시 한번 유의하세요.
// 여기서는 파일을 열고 배열로 읽어 들인 다음, 해당 배열 데이터를 올바른 순서로 m_targaData 배열에 로드합니다.
// 우리는 의도적으로 알파 채널이 있는 32비트 타가 파일만 다루며, 24비트로 저장된 타가는 이 함수에서 거부됩니다.
bool TextureClass::LoadTarga32Bit(HWND hwnd, char* filename)
{
	int error, bpp, imageSize, index, i, j, k;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;

	// 타가 파일을 바이너리 읽기 모드로 엽니다.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// 파일 헤더를 읽어 들입니다.
	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// 헤더에서 중요한 정보를 가져옵니다.
	m_height = (int)targaFileHeader.height;
	m_width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// 32비트인지 확인하고, 24비트이면 거부합니다.
	if (bpp != 32)
	{
		return false;
	}

	// 32비트 이미지 데이터의 크기를 계산합니다.
	imageSize = m_width * m_height * 4;

	// 타가 이미지 데이터를 위한 메모리를 할당합니다.
	targaImage = new unsigned char[imageSize];

	// 타가 이미지 데이터를 읽어 들입니다.
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// 파일을 닫습니다.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// 타가 목적지 데이터를 위한 메모리를 할당합니다.
	m_targaData = new unsigned char[imageSize];

	// 타가 목적지 데이터 배열의 인덱스를 초기화합니다.
	index = 0;

	// 타가 이미지 데이터의 인덱스를 초기화합니다.
	k = (m_width * m_height * 4) - (m_width * 4);

	// 이제 타가 이미지를 거꾸로 저장하고 RGBA 순서가 아니므로
	// 타가 이미지 데이터를 올바른 순서로 타가 목적지 배열에 복사합니다.
	for (j = 0; j < m_height; j++)
	{
		for (i = 0; i < m_width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2];  // 빨강(Red)
			m_targaData[index + 1] = targaImage[k + 1];  // 초록(Green)
			m_targaData[index + 2] = targaImage[k + 0];  // 파랑(Blue)
			m_targaData[index + 3] = targaImage[k + 3];  // 알파(Alpha)

			// 타가 데이터의 인덱스를 증가시킵니다.
			k += 4;
			index += 4;
		}

		// 거꾸로 읽고 있으므로, 타가 이미지 데이터 인덱스를
		// 이전 행의 시작 부분으로 되돌립니다.
		k -= (m_width * 8);
	}

	// 이제 타가 이미지 데이터가 목적지 배열에 복사되었으므로 해제합니다.
	delete[] targaImage;
	targaImage = 0;

	return true;
}


int TextureClass::GetWidth()
{
	return m_width;
}


int TextureClass::GetHeight()
{
	return m_height;
}