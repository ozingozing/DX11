///////////////////////////////////////////////////////////////////////////////
// Filename: fontclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "FontClass.h"

// 클래스 생성자는 FontClass의 모든 private 멤버 변수를 null로 초기화합니다.
FontClass::FontClass()
{
	m_Font = 0;
	m_Texture = 0;
}


FontClass::FontClass(const FontClass& other)
{
}


FontClass::~FontClass()
{
}

// 초기화 과정에서 글꼴 데이터와 글꼴 텍스처가 로드됩니다.
bool FontClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int fontChoice)
{
	char fontFilename[128];
	char fontTextureFilename[128];
	bool result;
	// 나중에 글꼴을 추가하여 쉽게 확장할 수 있도록 여기에 case 문을 넣어 두었습니다.
	// 현재로서는 이 튜토리얼에서 사용하는 첫 번째 글꼴만 선택할 수 있습니다.
	// 잘못된 fontChoice 값이 입력되면 항상 첫 번째 글꼴이 기본값으로 사용됩니다.

	// 제공되는 글꼴 중 하나를 선택하고, 그렇지 않으면 첫 번째 글꼴이 기본값으로 사용됩니다.
	switch (fontChoice)
	{
		case 0:
		{
			strcpy_s(fontFilename, "../Resource/font/font01.txt");
			strcpy_s(fontTextureFilename, "../Resource/font/font01.tga");
			m_fontHeight = 32.0f;
			m_spaceSize = 3;
			break;
		}
		default:
		{
			strcpy_s(fontFilename, "../Resource/font/font01.txt");
			strcpy_s(fontTextureFilename, "../Resource/font/font01.tga");
			m_fontHeight = 32.0f;
			m_spaceSize = 3;
			break;
		}
	}

	// 글꼴 데이터가 포함된 텍스트 파일을 불러옵니다.
	result = LoadFontData(fontFilename);
	if (!result)
	{
		return false;
	}
	// 글꼴 문자가 포함된 텍스처를 불러옵니다.
	result = LoadTexture(device, deviceContext, fontTextureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

// 종료하면 글꼴 데이터와 글꼴 텍스처가 해제됩니다.
void FontClass::Shutdown()
{
	// Release the font texture.
	ReleaseTexture();

	// Release the font data.
	ReleaseFontData();

	return;
}

// LoadFontData 함수는 텍스처의 인덱싱 정보가 포함된 font01.txt 파일을 불러오는 곳입니다.
bool FontClass::LoadFontData(char* filename)
{
	std::ifstream fin;
	int i;
	char temp;
	// 먼저 FontType 구조체의 배열을 생성합니다.
	// 배열의 크기는 텍스처의 문자 수이자 font01.txt 파일의 인덱스 수인 95로 설정합니다.

	// 글꼴 간격 버퍼를 생성합니다.
	m_Font = new FontType[95];

	// 이제 파일을 열고 각 줄을 m_Font 배열에 읽어들입니다.
	// 문자의 픽셀 크기와 함께 텍스처 TU의 왼쪽 및 오른쪽 좌표만 읽어들이면 됩니다.

	// 글꼴 크기와 문자 간 간격을 읽어옵니다.
	fin.open(filename);
	if (fin.fail())
	{
		return false;
	}

	// 텍스트에 사용되는 95개의 ASCII 문자를 읽어들입니다.
	for (i = 0; i < 95; i++)
	{
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}

	// Close the file.
	fin.close();

	return true;
}

// ReleaseFontData 함수는 텍스처 인덱싱 데이터를 담고 있는 배열을 해제합니다.
void FontClass::ReleaseFontData()
{
	// 글꼴 데이터 배열을 해제합니다.
	if (m_Font)
	{
		delete[] m_Font;
		m_Font = 0;
	}

	return;
}

// LoadTexture 함수는 font01.tga 파일을 텍스처 셰이더 리소스로 읽어들입니다.
// 이 텍스처에서 문자를 가져와 렌더링을 위해 각각의 정사각형 폴리곤에 기록합니다.
bool FontClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;

	// 글꼴 텍스처 객체를 생성하고 초기화합니다.
	m_Texture = new TextureClass;

	result = m_Texture->Initialize(device, deviceContext, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

// ReleaseTexture 함수는 글꼴에 사용된 텍스처를 해제합니다.
void FontClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}

// GetTexture는 글꼴 그래픽을 렌더링할 수 있도록 글꼴 텍스처를 반환합니다.
ID3D11ShaderResourceView* FontClass::GetTexture()
{
	return m_Texture->GetTexture();
}

// BuildVertexArray 함수는 TextClass에서 호출되어 입력으로 전달되는
// 텍스트 문장으로부터 정점 버퍼를 생성합니다.
// 이렇게 하면 TextClass에서 그려야 하는 문장은 생성 후
// 쉽게 렌더링할 수 있는 자체 정점 버퍼를 갖게 됩니다.
// vertices 입력값은 생성 후 TextClass에 반환될 정점 배열에 대한 포인터입니다.
// sentence 입력값은 정점 배열을 생성하는 데 사용될 텍스트 문장입니다.
// drawX 및 drawY 입력 변수는 문장을 그릴 화면 좌표입니다.
void FontClass::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr;
	int numLetters, index, i, letter;

	// 입력 정점을 VertexType 구조체로 변환합니다.
	vertexPtr = (VertexType*)vertices;

	// 문장의 글자 수를 구합니다.
	numLetters = (int)strlen(sentence);

	// 정점 배열의 인덱스를 초기화합니다.
	index = 0;
	// 다음 루프는 정점 및 인덱스 배열을 생성합니다.
	// 문장의 각 문자를 가져와 두 개의 삼각형을 생성합니다.
	// 그런 다음 글꼴 텍스처의 좌표와 픽셀 크기를 포함하는 m_Font 배열을 사용하여
	// 글꼴 텍스처의 문자를 이 두 삼각형에 매핑합니다.
	// 해당 문자에 대한 다각형이 생성되면 다음 문자를 그릴 위치의 화면 X 좌표를 업데이트합니다.

	// 각 글자를 사각형 위에 그립니다.
	for (i = 0; i < numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		// 글자가 공백이면 3픽셀만큼 이동합니다.
		if (letter == 0)
		{
			drawX = drawX + m_spaceSize;
		}
		else
		{
			// 사각형의 첫 번째 삼각형.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - m_fontHeight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - m_fontHeight), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 1.0f);
			index++;

			// 사각형의 두 번째 삼각형.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - m_fontHeight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			// 글자 크기와 1픽셀만큼 그림을 그릴 x 위치를 업데이트합니다.
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}
	return;
}

// GetSentencePixelLength 함수는 문장의 길이를 픽셀 단위로 반환합니다.
// 이 값을 사용하여 화면에서 텍스트 문장을 가운데 정렬하거나 다른 위치에 배치할 수 있습니다.
int FontClass::GetSentencePixelLength(char* sentence)
{
	int pixelLength, numLetters, i, letter;


	pixelLength = 0;
	numLetters = (int)strlen(sentence);

	for (i = 0; i < numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		// 글자가 공백이면 3픽셀로 계산합니다.
		if (letter == 0)
		{
			pixelLength += m_spaceSize;
		}
		else
		{
			pixelLength += (m_Font[letter].size + 1);
		}
	}

	return pixelLength;
}

// GetFontHeight 함수는 문장을 화면에 배치하는 데 도움이 되는 또 다른 함수입니다.
// GetSentencePixelLength에서 얻은 너비와 이 함수에서 얻은 높이를 사용하면
// 문장을 배치하는 데 필요한 모든 정보를 얻을 수 있습니다.
int FontClass::GetFontHeight()
{
	return (int)m_fontHeight;
}