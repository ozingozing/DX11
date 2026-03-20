#include "LightShaderClass.h"


LightShaderClass::LightShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_sampleState = 0;
    m_matrixBuffer = 0;

    // 클래스 생성자에서 새로운 조명 상수 버퍼를 null로 초기화합니다.
    m_cameraBuffer = 0;
    m_lightBuffer = 0;
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
    ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor,
    XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    bool result;


    // 렌더링에 사용할 셰이더 매개변수를 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambientColor, diffuseColor,
        cameraPosition, specularColor, specularPower);
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
    D3D11_BUFFER_DESC cameraBufferDesc;
    // 조명 상수 버퍼를 위한 새로운 기술서(description) 변수를 추가합니다.
    D3D11_BUFFER_DESC lightBufferDesc;


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

    // 새 카메라 버퍼에 대한 설명을 설정한 다음, 해당 설명을 사용하여 버퍼를 생성합니다.
    // 이렇게 하면 정점 셰이더에서 카메라 위치를 설정하고 상호 작용할 수 있습니다.

    // 정점 셰이더에 있는 카메라 동적 상수 버퍼에 대한 설명을 설정합니다.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

    // 이 클래스 내부에서 정점 셰이더 상수 버퍼에 접근할 수 있도록 카메라 상수 버퍼 포인터를 생성합니다.
	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 여기에서 확산 조명 색상과 조명 방향을 처리할 조명 상수 버퍼 기술서를 설정합니다.
    // 상수 버퍼의 크기에 주의하십시오. 16의 배수가 아니라면 끝에 추가 공간을 패딩해야 하며, 그렇지 않으면 CreateBuffer 함수가 실패합니다.
    // 이 경우 상수 버퍼는 28바이트이며, 32바이트를 맞추기 위해 4바이트의 패딩이 추가되었습니다.

    // 픽셀 셰이더에 있는 조명 동적 상수 버퍼의 기술서를 설정합니다.
    // D3D11_BIND_CONSTANT_BUFFER를 사용할 때 ByteWidth는 항상 16의 배수여야 하며, 그렇지 않으면 CreateBuffer가 실패합니다.
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // 클래스 내에서 정점 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void LightShaderClass::ShutdownShader()
{
    // Release the camera constant buffer.
    if (m_cameraBuffer)
    {
        m_cameraBuffer->Release();
        m_cameraBuffer = 0;
    }
    // 조명 상수 버퍼를 해제합니다.
    if (m_lightBuffer)
    {
        m_lightBuffer->Release();
        m_lightBuffer = 0;
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
    ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor,
    XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    unsigned int bufferNumber;
    MatrixBufferType* dataPtr;
    LightBufferType* dataPtr2;
	CameraBufferType* dataPtr3;

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

    //여기서는 카메라 버퍼를 잠그고 그 안에 카메라 위치 값을 설정합니다.

    // Lock the camera constant buffer so it can be written to.
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 카메라 상수 버퍼를 잠가 쓰기 작업이 가능하도록 합니다.
    dataPtr3 = (CameraBufferType*)mappedResource.pData;

    // 카메라 위치를 상수 버퍼에 복사합니다.
    dataPtr3->cameraPosition = cameraPosition;
    dataPtr3->padding = 0.0f;

    // 카메라 상수 버퍼의 잠금을 해제합니다.
    deviceContext->Unmap(m_cameraBuffer, 0);

    // 조명 상수 버퍼는 행렬 상수 버퍼와 동일한 방식으로 설정됩니다.
    // 먼저 버퍼를 잠그고 포인터를 얻습니다. 그 후 포인터를 사용하여 확산색과 조명 방향을 설정합니다.
    // 데이터 설정이 완료되면 버퍼의 잠금을 해제하고 픽셀 셰이더에 설정합니다.
    // 이때는 정점 셰이더 버퍼가 아닌 픽셀 셰이더 버퍼를 설정하는 것이므로 VSSetConstantBuffers 대신 PSSetConstantBuffers 함수를 사용함에 유의하십시오.

    // 조명 상수 버퍼를 쓸 수 있도록 잠급니다.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼를 설정하기 전에 bufferNumber를 0이 아닌 1로 설정하는 것에 유의하십시오.
    // 이는 정점 셰이더에서 두 번째 버퍼이기 때문입니다(첫 번째는 행렬 버퍼).
    // 정점 셰이더에서 카메라 상수 버퍼의 위치를 ​​설정합니다.
    bufferNumber = 1;

    // 이제 정점 셰이더에서 카메라 상수 버퍼를 업데이트된 값으로 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // Lock the light constant buffer so it can be written to.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 데이터에 대한 포인터를 얻습니다.
    dataPtr2 = (LightBufferType*)mappedResource.pData;

    // 조명 변수들을 상수 버퍼로 복사합니다.
    dataPtr2->ambientColor = ambientColor;
    dataPtr2->diffuseColor = diffuseColor;
    dataPtr2->lightDirection = lightDirection;
    dataPtr2->specularColor = specularColor;
    dataPtr2->specularPower = specularPower;

    // 상수 버퍼의 잠금을 해제합니다.
    deviceContext->Unmap(m_lightBuffer, 0);

    // 픽셀 셰이더 내에서 조명 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 1; // 버퍼 저장위치 b1이기 떄문에 1로

    // 마지막으로 픽셀 셰이더의 조명 상수 버퍼를 업데이트된 값들로 설정합니다.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

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