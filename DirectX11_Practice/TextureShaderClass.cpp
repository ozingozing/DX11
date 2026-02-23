////////////////////////////////////////////////////////////////////////////////
// 파일명: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureshaderclass.h"


// 클래스 생성자는 모든 포인터 변수를 null로 초기화합니다.
TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	// 새로운 샘플러 변수가 클래스 생성자에서 null로 설정됩니다.
	m_sampleState = 0;
}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


// Initialize 함수는 텍스처 셰이더를 초기화합니다.
bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;
	// 새로운 texture.vs와 texture.ps HLSL 파일들이 이 셰이더를 위해 로드됩니다.
		// 버텍스 셰이더 파일명을 설정합니다.
	error = wcscpy_s(vsFilename, 128, L"Texture.hlsl");
	if (error != 0)
	{
		return false;
	}

	// 픽셀 셰이더 파일명을 설정합니다.
	error = wcscpy_s(psFilename, 128, L"Texture.hlsl");
	if (error != 0)
	{
		return false;
	}

	// 버텍스 및 픽셀 셰이더를 초기화합니다.
	result = InitializeShader(device, hwnd, vsFilename, psFilename);
	if (!result)
	{
		return false;
	}

	return true;
}
// Shutdown 함수는 셰이더 변수들을 해제하는 함수를 호출합니다.
void TextureShaderClass::Shutdown()
{
	// 버텍스 및 픽셀 셰이더와 관련 객체들을 종료합니다.
	ShutdownShader();

	return;
}

// Render 함수는 이제 텍스처 리소스에 대한 포인터인 texture라는 새로운 매개변수를 받습니다.
// 이 포인터는 셰이더가 텍스처를 설정하고 렌더링에 사용할 수 있도록 SetShaderParameters 함수로 전달됩니다.
bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;

	// 렌더링에 사용할 셰이더 매개변수를 설정합니다.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result)
	{
		return false;
	}

	// 이제 셰이더를 사용하여 준비된 버퍼를 렌더링합니다.
	RenderShader(deviceContext, indexCount);

	return true;
}

// InitializeShader 함수는 텍스처 셰이더를 설정합니다.
bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	// 이 함수에서 설정될 텍스처 샘플러의 설명을 담을 새로운 변수입니다.
	D3D11_SAMPLER_DESC samplerDesc;

	// 이 함수에서 사용할 포인터들을 null로 초기화합니다.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;
	// 새로운 텍스처 버텍스 및 픽셀 셰이더를 로드합니다.
		// 버텍스 셰이더 코드를 컴파일합니다.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일에 실패하면 오류 메시지가 작성되었을 것입니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// 오류 메시지가 없으면 셰이더 파일을 찾지 못한 것입니다.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 픽셀 셰이더 코드를 컴파일합니다.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일에 실패하면 오류 메시지가 작성되었을 것입니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// 오류 메시지가 없으면 파일을 찾지 못한 것입니다.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 버퍼로부터 버텍스 셰이더를 생성합니다.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// 버퍼로부터 픽셀 셰이더를 생성합니다.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 색상 대신 텍스처 요소를 가지므로 입력 레이아웃이 변경되었습니다.
	// 첫 번째 위치 요소는 변경되지 않았지만, 두 번째 요소의 SemanticName과 Format이 TEXCOORD와 DXGI_FORMAT_R32G32_FLOAT로 변경되었습니다.
	// 이 두 가지 변경으로 ModelClass 정의와 셰이더 파일의 typedefs에 있는 새로운 VertexType과 이 레이아웃이 일치하게 됩니다.
		// 버텍스 입력 레이아웃 설명을 생성합니다.
		// 이 설정은 ModelClass의 VertexType 구조체 및 셰이더의 구조체와 일치해야 합니다.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// 레이아웃의 요소 개수를 얻습니다.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 버텍스 입력 레이아웃을 생성합니다.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// 더 이상 필요하지 않으므로 버텍스 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// 버텍스 셰이더에 있는 동적 행렬 상수 버퍼의 설명을 설정합니다.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 버텍스 셰이더의 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 여기서 샘플러 상태 설명을 설정한 후 픽셀 셰이더에 전달할 수 있습니다.
	// 텍스처 샘플러 설명에서 가장 중요한 요소는 Filter입니다. Filter는 폴리곤 면에 텍스처의 최종 모습을 만들기 위해 어떤 픽셀을 사용하거나 결합할지를 결정합니다.
	// 여기서는 처리 비용이 더 많이 들지만 최고의 시각적 결과를 제공하는 D3D11_FILTER_MIN_MAG_MIP_LINEAR를 사용합니다.
	// 이는 샘플러에게 축소(minification), 확대(magnification), 밉 레벨 샘플링에 대해 선형 보간(linear interpolation)을 사용하도록 지시합니다.
	// AddressU와 AddressV는 Wrap으로 설정되어 좌표가 0.0f와 1.0f 사이에 머무르도록 보장합니다. 이 범위를 벗어나는 값은 감싸져서 0.0f와 1.0f 사이에 위치하게 됩니다.
	// 샘플러 상태 설명의 다른 모든 설정은 기본값입니다.
		// 텍스처 샘플러 상태 설명을 생성합니다.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// 텍스처 샘플러 상태를 생성합니다.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
// ShutdownShader 함수는 TextureShaderClass에서 사용된 모든 변수들을 해제합니다.
void TextureShaderClass::ShutdownShader()
{
	// ShutdownShader 함수는 이제 초기화 중에 생성된 새로운 샘플러 상태를 해제합니다.
		// 샘플러 상태를 해제합니다.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// 행렬 상수 버퍼를 해제합니다.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// 레이아웃을 해제합니다.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// 픽셀 셰이더를 해제합니다.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// 버텍스 셰이더를 해제합니다.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}
// OutputShaderErrorMessage는 HLSL 셰이더를 로드할 수 없을 때 오류를 텍스트 파일에 기록합니다.
void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	ofstream fout;


	// 오류 메시지 텍스트 버퍼에 대한 포인터를 가져옵니다.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// 메시지의 길이를 가져옵니다.
	bufferSize = errorMessage->GetBufferSize();

	// 오류 메시지를 기록할 파일을 엽니다.
	fout.open("shader-error.txt");

	// 오류 메시지를 기록합니다.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// 파일을 닫습니다.
	fout.close();

	// 오류 메시지를 해제합니다.
	errorMessage->Release();
	errorMessage = 0;

	// 사용자에게 컴파일 오류를 확인하도록 텍스트 파일 확인을 알리는 메시지 창을 띄웁니다.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}
// SetShaderParameters 함수는 이제 텍스처 리소스에 대한 포인터를 입력으로 받아서,
// 새로운 텍스처 리소스 포인터를 사용하여 셰이더에 할당합니다.
// 버퍼 렌더링이 발생하기 전에 텍스처가 설정되어야 함에 유의하세요.
bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// 셰이더를 준비하기 위해 행렬들을 전치(Transpose)합니다.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// 쓰기 가능하도록 상수 버퍼를 잠급니다.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 행렬들을 상수 버퍼에 복사합니다.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// 상수 버퍼를 잠금 해제합니다.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// 버텍스 셰이더에서 상수 버퍼의 위치를 설정합니다.
	bufferNumber = 0;

	// 최종적으로 업데이트된 값을 가진 상수 버퍼를 버텍스 셰이더에 설정합니다.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	// SetShaderParameters 함수는 이제 픽셀 셰이더에 텍스처를 설정하는 기능을 포함하도록 이전 튜토리얼에서 수정되었습니다.
		// 픽셀 셰이더에 셰이더 텍스처 리소스를 설정합니다.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}
// RenderShader는 폴리곤을 렌더링하기 위해 셰이더 테크닉을 호출합니다.
void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// 버텍스 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);

	// 이 삼각형을 렌더링하는 데 사용될 버텍스 및 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	// RenderShader 함수는 렌더링하기 전에 픽셀 셰이더에 샘플러 상태를 설정하는 기능이 추가되었습니다.
		// 픽셀 셰이더에 샘플러 상태를 설정합니다.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// 삼각형을 렌더링합니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}