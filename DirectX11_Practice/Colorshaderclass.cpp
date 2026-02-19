////////////////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "colorshaderclass.h"
// 평소와 같이 클래스 생성자는 클래스 내의 모든 private 포인터를 null로 초기화합니다.

ColorShaderClass::ColorShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
}


ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
{
}


ColorShaderClass::~ColorShaderClass()
{
}

// Initialize 함수는 셰이더의 초기화 함수를 호출합니다.
// 이 함수에는 HLSL 셰이더 파일의 이름을 전달하는데,
// 이 튜토리얼에서는 color.hlsl라는 이름입니다.
bool ColorShaderClass::Initialize(ID3D11Device * device, HWND hwnd)
{
	bool result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;


	// Set the filename of the vertex shader.
	error = wcscpy_s(vsFilename, 128, L"Color.hlsl");
	if (error != 0)
	{
		return false;
	}

	// Set the filename of the pixel shader.
	error = wcscpy_s(psFilename, 128, L"Color.hlsl");
	if (error != 0)
	{
		return false;
	}

	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, vsFilename, psFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

// Shutdown 함수는 셰이더의 종료를 호출합니다.
void ColorShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

// Render 함수는 먼저 SetShaderParameters 함수를 사용하여 셰이더 내부의 매개변수를 설정합니다.
// 매개변수가 설정되면 RenderShader 함수를 호출하여 HLSL 셰이더를 사용하여 녹색 삼각형을 그립니다.
bool ColorShaderClass::Render(
	ID3D11DeviceContext* deviceContext,
	int indexCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

// 이제 이 튜토리얼에서 가장 중요한 함수 중 하나인 InitializeShader 함수부터 시작하겠습니다.
// 이 함수는 실제로 셰이더 파일을 로드하여 DirectX와 GPU에서 사용할 수 있도록 만듭니다.
// 또한 레이아웃 설정과 GPU의 그래픽 파이프라인에서 정점 버퍼 데이터가 어떻게 표시되는지도 확인할 수 있습니다.
// 레이아웃은 modelclass.h 파일과 color.hlsl 파일에 정의된 VertexType과 일치해야 합니다.
bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;


	// 함수에서 사용할 포인터들을 null로 초기화합니다.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// 여기서 셰이더 프로그램을 버퍼로 컴파일합니다.
	// 셰이더 파일 이름, 셰이더 이름, 셰이더 버전(DirectX 11에서는 5.0),
	// 그리고 셰이더를 컴파일할 버퍼의 이름을 지정합니다.
	// 셰이더 컴파일에 실패하면 errorMessage 문자열에 오류 메시지가 저장되고,
	// 이 문자열을 다른 함수로 전달하여 오류를 출력합니다. 만약 여전히 컴파일에 실패하고
	// errorMessage 문자열도 없다면, 셰이더 파일을 찾을 수 없다는 의미이며,
	// 이 경우 오류 메시지를 표시하는 대화 상자를 띄웁니다.

	// 정점 셰이더 코드를 컴파일합니다.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일에 실패했다면 오류 메시지에 어떤 내용이 적혀 있어야 합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// 오류 메시지에 아무런 내용이 없다면 셰이더 파일을 찾을 수 없는 것입니다.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 픽셀 셰이더 코드를 컴파일합니다.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// 셰이더 컴파일에 실패했다면 오류 메시지에 어떤 내용이 적혀 있어야 합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// 오류 메시지에 아무런 내용이 없다면, 단순히 파일을 찾을 수 없는 것입니다.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 정점 셰이더와 픽셀 셰이더 코드가 버퍼로 성공적으로 컴파일되면,
	// 해당 버퍼를 사용하여 셰이더 객체 자체를 생성합니다.
	// 앞으로 우리는 이러한 포인터를 사용하여 정점 셰이더 및 픽셀 셰이더와 상호 작용할 것입니다.

	// 버퍼로부터 정점 셰이더를 생성합니다.
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

	// 다음 단계는 셰이더에서 처리할 정점 데이터의 레이아웃을 생성하는 것입니다.
	// 이 셰이더는 위치(Position)와 색상(Color) 벡터를 사용하므로, 레이아웃에 두 요소의 크기를 각각 지정해야 합니다.

	// 1. Semantic Name (세만틱 이름): 레이아웃에서 가장 먼저 채워야 할 항목으로, 셰이더가 이 원소의 용도를 파악하게 합니다.
	//    두 가지 요소를 사용하므로 첫 번째는 'POSITION', 두 번째는 'COLOR'를 사용합니다.

	// 2. Format (형식): 각 벡터의 데이터 크기를 지정합니다.
	//    위치 벡터에는 DXGI_FORMAT_R32G32B32_FLOAT를, 색상에는 DXGI_FORMAT_R32G32B32A32_FLOAT를 사용합니다.

	// 3. AlignedByteOffset (정렬 바이트 오프셋): 버퍼 내에서 데이터가 어떻게 배치되는지 나타냅니다.
	//    이 레이아웃에서는 처음 12바이트가 위치 정보이고, 그다음 16바이트가 색상 정보임을 알려줍니다.
	//    직접 수치를 입력하는 대신 D3D11_APPEND_ALIGNED_ELEMENT를 사용하면 시스템이 자동으로 간격을 계산해 줍니다.

	// 그 외 설정값들은 이번 튜토리얼에서 필요하지 않으므로 기본값(Default)을 사용합니다.

	// 정점 입력 레이아웃 설정을 생성합니다.
	// 이 설정은 ModelClass의 VertexType 구조체 및 셰이더의 구조와 일치해야 합니다.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// 레이아웃 설정이 완료되면, 요소의 개수를 구한 뒤 Direct3D 장치를 사용하여 입력 레이아웃을 생성합니다.
	// 레이아웃이 생성된 후에는 더 이상 필요 없는 정점 및 픽셀 셰이더 버퍼를 해제합니다.

	// 레이아웃의 요소 개수를 가져옵니다.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 생성합니다.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// 더 이상 사용하지 않는 정점 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// 셰이더를 활용하기 위해 마지막으로 설정해야 할 것은 상수 버퍼(Constant Buffer)입니다.
	// 정점 셰이더에서 보았듯이 현재 하나의 상수 버퍼만 사용하므로,
	// 여기서도 하나만 설정하여 셰이더와 통신할 수 있게 합니다.
	// 매 프레임마다 업데이트할 예정이므로 Usage는 Dynamic으로 설정합니다.
	// BindFlags는 이 버퍼가 상수 버퍼임을 나타내며,
	// CPUAccessFlags는 Usage에 맞춰 D3D11_CPU_ACCESS_WRITE로 설정해야 합니다.
	// 설명을 모두 채운 뒤 상수 버퍼 인터페이스를 생성하고,
	// 이후 SetShaderParameters 함수를 통해 셰이더 내부 변수에 접근합니다.

	// 정점 셰이더에 있는 동적 행렬 상수 버퍼의 설명을 설정합니다.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 정점 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

// ShutdownShader 함수는 InitializeShader 함수에서 설정했던 4개의 인터페이스를 해제합니다.
void ColorShaderClass::ShutdownShader()
{
	// 행렬 상수 버퍼를 해제합니다.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// 레이아웃 객체를 해제합니다.
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

	// 정점 셰이더를 해제합니다.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

// OutputShaderErrorMessage 함수는 정점 셰이더나 픽셀 셰이더 컴파일 시 발생하는 오류 메시지를 기록합니다.
void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

	// 오류 메시지를 파일에 기록합니다.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// 파일을 닫습니다.
	fout.close();

	// 오류 메시지 리소스를 해제합니다.
	errorMessage->Release();
	errorMessage = 0;

	// 컴파일 오류 확인을 위해 텍스트 파일을 확인하라는 메시지 박스를 화면에 띄웁니다.
	MessageBox(hwnd, L"셰이더 컴파일 중 오류가 발생했습니다. shader-error.txt 파일을 확인하세요.", shaderFilename, MB_OK);

	return;
}

// SetShaderParameters 함수는 셰이더의 전역 변수들을 쉽게 설정하기 위해 존재합니다.
// 이 함수에서 사용하는 행렬들은 ApplicationClass에서 생성되며, 
// Render 함수가 호출되는 동안 이 함수를 통해 정점 셰이더로 전달됩니다.
bool ColorShaderClass::SetShaderParameters(
	ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// 행렬을 셰이더로 보내기 전에 전치(Transpose)해야 합니다. 이는 DirectX 11의 필수 사항입니다.

	// 셰이더에서 사용할 수 있도록 행렬을 전치합니다.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// m_matrixBuffer를 잠그고(Lock), 새로운 행렬들을 설정한 뒤, 다시 잠금을 해제(Unlock)합니다.

	// 쓰기 가능하도록 상수 버퍼를 잠급니다.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 행렬들을 상수 버퍼로 복사합니다.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// 상수 버퍼의 잠금을 해제합니다.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// 이제 HLSL 정점 셰이더에서 업데이트된 행렬 버퍼를 설정합니다.

	// 정점 셰이더 내 상수 버퍼의 위치를 지정합니다.
	bufferNumber = 0;

	// 마지막으로, 업데이트된 값들이 담긴 상수 버퍼를 정점 셰이더에 적용합니다.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}

// RenderShader는 Render 함수 내에서 두 번째로 호출되는 함수입니다.
// 셰이더 매개변수들이 올바르게 설정되었는지 확인하기 위해, 이 함수 이전에 SetShaderParameters가 먼저 호출되어야 합니다.

// 이 함수의 첫 번째 단계는 입력 어셈블러(Input Assembler)에 입력 레이아웃을 활성 상태로 설정하는 것입니다.
// 이를 통해 GPU는 정점 버퍼에 담긴 데이터의 형식을 알게 됩니다.
// 두 번째 단계는 이 정점 버퍼를 렌더링하는 데 사용할 정점 셰이더와 픽셀 셰이더를 설정하는 것입니다.
// 셰이더 설정이 완료되면, D3D 디바이스 컨텍스트의 DrawIndexed 함수를 호출하여 삼각형을 렌더링합니다.
// 이 함수가 실행되면 화면에 초록색 삼각형이 그려지게 됩니다.
void ColorShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout);

	// 이 삼각형을 렌더링하는 데 사용할 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 삼각형을 렌더링합니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}