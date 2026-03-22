#include "LightShaderClass.h"


LightShaderClass::LightShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_sampleState = 0;
    m_matrixBuffer = 0;

    m_lightColorBuffer = 0;
    m_lightPositionBuffer = 0;
}


LightShaderClass::LightShaderClass(const LightShaderClass& other)
{
}


LightShaderClass::~LightShaderClass()
{
}


bool LightShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;
    bool result;

    // 조명 셰이더를 초기화하기 위해 새로운 light.vs와 light.ps HLSL 셰이더 파일을 입력으로 사용합니다.

    // 정점 셰이더 파일의 이름을 설정합니다.
    error = wcscpy_s(vsFilename, 128, L"Light.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더 파일의 이름을 설정합니다.
    error = wcscpy_s(psFilename, 128, L"Light.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 정점 및 픽셀 셰이더를 초기화합니다.
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


void LightShaderClass::Shutdown()
{
    // 관련 객체들뿐만 아니라 정점 및 픽셀 셰이더를 종료합니다.
    ShutdownShader();

    return;
}

// Render 함수는 이제 조명 방향(light direction)과 조명 확산색(diffuse color)을 입력으로 받습니다.
// 이 변수들은 SetShaderParameters 함수로 전달되어 최종적으로 셰이더 내부에서 설정됩니다.
bool LightShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture, XMFLOAT4 diffuseColor[], XMFLOAT4 lightPosition[])
{
    bool result;


    // 렌더링에 사용할 셰이더 매개변수를 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, diffuseColor, lightPosition);
    if (!result)
    {
        return false;
    }

    // 준비된 버퍼들을 셰이더를 통해 렌더링합니다.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool LightShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;

    // polygonLayout 변수가 2개에서 3개의 요소로 변경되었습니다.
    // 이는 레이아웃에 법선 벡터(normal vector)를 수용하기 위함입니다.
    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
    unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC lightColorBufferDesc;
    D3D11_BUFFER_DESC lightPositionBufferDesc;


    // 이 함수에서 사용할 포인터들을 null로 초기화합니다.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 새로운 조명 정점 셰이더를 로드합니다.

    // 정점 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패했다면 에러 메시지에 무언가 기록되었을 것입니다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // 에러 메시지에 아무것도 없다면 셰이더 파일 자체를 찾을 수 없는 경우입니다.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // 새로운 조명 픽셀 셰이더를 로드합니다.

    // 픽셀 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패했다면 에러 메시지에 무언가 기록되었을 것입니다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // 에러 메시지에 아무것도 없다면 파일 자체를 찾을 수 없는 경우입니다.
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

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

    // 정점 입력 레이아웃 기술서를 생성합니다.
    // 이 설정은 ModelClass와 셰이더 내부의 VertexType 구조체와 일치해야 합니다.
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

    // 셰이더 초기화에서 가장 큰 변경점 중 하나가 이 polygonLayout 부분입니다.
    // 조명에 사용할 법선 벡터를 위해 세 번째 요소를 추가합니다.
    // 시맨틱 이름은 NORMAL이며 포맷은 법선 벡터의 x, y, z를 처리하는 일반적인 DXGI_FORMAT_R32G32B32_FLOAT를 사용합니다.
    // 이제 이 레이아웃은 HLSL 정점 셰이더에서 기대하는 입력값과 일치하게 됩니다.
    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // 레이아웃의 요소 개수를 구합니다.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 정점 입력 레이아웃을 생성합니다.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
        &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // 더 이상 필요 없는 정점 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제합니다.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 텍스처 샘플러 상태 기술서를 생성합니다.
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

    // 정점 셰이더에 있는 동적 행렬 상수 버퍼의 기술서를 설정합니다.
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

    // Setup the description of the dytnamic constant buffer that is in the pixel shader
	lightColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightColorBufferDesc.ByteWidth = sizeof(LightColorBufferType);
	lightColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightColorBufferDesc.MiscFlags = 0;
	lightColorBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class
	result = device->CreateBuffer(&lightColorBufferDesc, NULL, &m_lightColorBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Setup the description of the dynamic constant buffer that is in the vertex shader.
    lightPositionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightPositionBufferDesc.ByteWidth = sizeof(LightPositionBufferType);
    lightPositionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightPositionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightPositionBufferDesc.MiscFlags = 0;
    lightPositionBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&lightPositionBufferDesc, NULL, &m_lightPositionBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void LightShaderClass::ShutdownShader()
{
    // Release the light constant buffers.
    if (m_lightColorBuffer)
    {
        m_lightColorBuffer->Release();
        m_lightColorBuffer = 0;
    }

    if (m_lightPositionBuffer)
    {
        m_lightPositionBuffer->Release();
        m_lightPositionBuffer = 0;
    }

    // 행렬 상수 버퍼를 해제합니다.
    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    // 샘플러 상태를 해제합니다.
    if (m_sampleState)
    {
        m_sampleState->Release();
        m_sampleState = 0;
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

    // 정점 셰이더를 해제합니다.
    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }

    return;
}


void LightShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned __int64 bufferSize, i;
    ofstream fout;


    // 에러 메시지 텍스트 버퍼에 대한 포인터를 얻습니다.
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // 메시지의 길이를 얻습니다.
    bufferSize = errorMessage->GetBufferSize();

    // 에러 메시지를 기록할 파일을 엽니다.
    fout.open("shader-error.txt");

    // 에러 메시지를 씁니다.
    for (i = 0; i < bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // 파일을 닫습니다.
    fout.close();

    // 에러 메시지를 해제합니다.
    errorMessage->Release();
    errorMessage = 0;

    // 사용자에게 텍스트 파일에서 컴파일 에러를 확인하라는 메시지를 화면에 띄웁니다.
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}

// SetShaderParameters 함수는 이제 lightDirection과 diffuseColor를 입력으로 받습니다.
bool LightShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture, XMFLOAT4 diffuseColor[], XMFLOAT4 lightPosition[])
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    unsigned int bufferNumber;
    MatrixBufferType* dataPtr;
    LightPositionBufferType* dataPtr2;
    LightColorBufferType* dataPtr3;

    // 행렬을 셰이더에서 사용할 수 있도록 전치(Transpose)합니다.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 상수 버퍼를 쓸 수 있도록 잠급니다(Lock).
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 데이터에 대한 포인터를 얻습니다.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // 행렬을 상수 버퍼로 복사합니다.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // 상수 버퍼의 잠금을 해제합니다(Unlock).
    deviceContext->Unmap(m_matrixBuffer, 0);

    // 정점 셰이더 내에서 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 0;

    // 이제 정점 셰이더의 상수 버퍼를 업데이트된 값들로 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // 쓰기 작업이 가능하도록 조명 위치 상수 버퍼를 잠급니다.
    result = deviceContext->Map(m_lightPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼에 있는 데이터에 대한 포인터를 가져옵니다.
    dataPtr2 = (LightPositionBufferType*)mappedResource.pData;

    // 광원 위치 변수를 상수 버퍼에 복사합니다.
    dataPtr2->lightPosition[0] = lightPosition[0];
    dataPtr2->lightPosition[1] = lightPosition[1];
    dataPtr2->lightPosition[2] = lightPosition[2];
    dataPtr2->lightPosition[3] = lightPosition[3];

    // 상수 버퍼의 잠금을 해제합니다.
    deviceContext->Unmap(m_lightPositionBuffer, 0);

    // 정점 셰이더에서 상수 버퍼의 위치를 ​​설정합니다.
    bufferNumber = 1;

    // 마지막으로, 정점 셰이더의 상수 버퍼를 업데이트된 값으로 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightPositionBuffer);

    // 픽셀 셰이더에서 셰이더 텍스처 리소스를 설정합니다.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // Lock the light color constant buffer so it can be written to.
    result = deviceContext->Map(m_lightColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr3 = (LightColorBufferType*)mappedResource.pData;

    // Copy the light color variables into the constant buffer.
    dataPtr3->diffuseColor[0] = diffuseColor[0];
    dataPtr3->diffuseColor[1] = diffuseColor[1];
    dataPtr3->diffuseColor[2] = diffuseColor[2];
    dataPtr3->diffuseColor[3] = diffuseColor[3];

    // Unlock the constant buffer.
    deviceContext->Unmap(m_lightColorBuffer, 0);

    // Set the position of the constant buffer in the pixel shader.
    bufferNumber = 2;

    // Finally set the constant buffer in the pixel shader with the updated values.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightColorBuffer);

    return true;
}


void LightShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 정점 입력 레이아웃을 설정합니다.
    deviceContext->IASetInputLayout(m_layout);

    // 이 삼각형을 렌더링하는 데 사용할 정점 및 픽셀 셰이더를 설정합니다.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에서 샘플러 상태를 설정합니다.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 삼각형을 렌더링합니다.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}