////////////////////////////////////////////////////////////////////////////////
// 파일명: clipplaneshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ClipPlaneShaderClass.h"


ClipPlaneShaderClass::ClipPlaneShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;

    // 클래스 생성자에서 클립 평면 버퍼를 null로 초기화합니다.
    m_clipPlaneBuffer = 0;
}


ClipPlaneShaderClass::ClipPlaneShaderClass(const ClipPlaneShaderClass& other)
{
}


ClipPlaneShaderClass::~ClipPlaneShaderClass()
{
}


bool ClipPlaneShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // 여기서는 clipplane.vs 및 clipplane.ps HLSL 셰이더 파일을 로드합니다.

    // 버텍스 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(vsFilename, 128, L"ClipPlane.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(psFilename, 128, L"ClipPlane.hlsl");
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


void ClipPlaneShaderClass::Shutdown()
{
    // 버텍스 및 픽셀 셰이더와 관련 객체들을 해제합니다.
    ShutdownShader();

    return;
}

// Render 함수는 이제 인자로 4개 float 벡터 형태의 클립 평면도 함께 받습니다. 
// 이 클립 평면은 렌더링하기 전에 셰이더에 설정됩니다.
bool ClipPlaneShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 clipPlane)
{
    bool result;


    // 렌더링에 사용할 셰이더 파라미터를 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, clipPlane);
    if (!result)
    {
        return false;
    }

    // 이제 준비된 버퍼를 셰이더와 함께 렌더링합니다.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool ClipPlaneShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC clipPlaneBufferDesc;


    // 이 함수가 사용할 포인터들을 null로 초기화합니다.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 클립 평면 버텍스 셰이더를 로드합니다.

    // 버텍스 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "ClipPlaneVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패한 경우 오류 메시지에 내용이 기록되어 있어야 합니다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // 오류 메시지가 비어 있다면 단순히 셰이더 파일 자체를 찾지 못한 경우입니다.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // 클립 평면 픽셀 셰이더를 로드합니다.

    // 픽셀 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(psFilename, NULL, NULL, "ClipPlanePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패한 경우 오류 메시지에 내용이 기록되어 있어야 합니다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // 오류 메시지가 비어 있다면 단순히 파일 자체를 찾지 못한 경우입니다.
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

    // 버텍스 입력 레이아웃 설명을 생성합니다.
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

    // 레이아웃의 요소 개수를 구합니다.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 버텍스 입력 레이아웃을 생성합니다.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // 더 이상 필요 없어진 버텍스 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제합니다.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 버텍스 셰이더에 있는 동적 매트릭스 상수 버퍼의 설명을 설정합니다.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // 이 클래스 내에서 버텍스 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

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

    // 클립 평면 버퍼를 설정합니다.

    // 버텍스 셰이더에 있는 동적 클립 평면 상수 버퍼의 설명을 설정합니다.
    clipPlaneBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    clipPlaneBufferDesc.ByteWidth = sizeof(ClipPlaneBufferType);
    clipPlaneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    clipPlaneBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    clipPlaneBufferDesc.MiscFlags = 0;
    clipPlaneBufferDesc.StructureByteStride = 0;

    // 이 클래스 내에서 버텍스 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&clipPlaneBufferDesc, NULL, &m_clipPlaneBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void ClipPlaneShaderClass::ShutdownShader()
{
    // ShutdownShader 함수에서 새로운 클립 평면 버퍼를 해제합니다.

    // 클립 평면 상수 버퍼를 해제합니다.
    if (m_clipPlaneBuffer)
    {
        m_clipPlaneBuffer->Release();
        m_clipPlaneBuffer = 0;
    }

    // 샘플러 상태를 해제합니다.
    if (m_sampleState)
    {
        m_sampleState->Release();
        m_sampleState = 0;
    }

    // 매트릭스 상수 버퍼를 해제합니다.
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


void ClipPlaneShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

    // 컴파일 오류에 대한 메시지를 확인하라는 알림창을 화면에 띄웁니다.
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}


bool ClipPlaneShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 clipPlane)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    ClipPlaneBufferType* dataPtr2;


    // 셰이더에 전달할 수 있도록 매트릭스들을 전치(Transpose)합니다.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 상수 버퍼에 쓸 수 있도록 락(Lock)을 겁니다.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내 데이터에 대한 포인터를 얻습니다.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // 매트릭스들을 상수 버퍼에 복사합니다.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // 상수 버퍼의 락을 풉니다(Unmap).
    deviceContext->Unmap(m_matrixBuffer, 0);

    // 버텍스 셰이더에서 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 0;

    // 업데이트된 값들로 버텍스 셰이더의 상수 버퍼를 최종 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // 픽셀 셰이더에 셰이더 텍스처 리소스를 설정합니다.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // 여기서는 클립 평면 상수 버퍼를 락하고, 클립 평면 값을 설정한 다음 다시 언락합니다. 
    // 이를 통해 셰이더 내부의 클립 평면 데이터가 설정됩니다. 
    // 또한 이는 버텍스 셰이더의 두 번째 버퍼이므로 bufferNumber를 0에서 1로 증가시켜야 한다는 점에 유의하세요.

    // 클립 평면 상수 버퍼에 쓸 수 있도록 락을 겁니다.
    result = deviceContext->Map(m_clipPlaneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 클립 평면 상수 버퍼 내 데이터에 대한 포인터를 얻습니다.
    dataPtr2 = (ClipPlaneBufferType*)mappedResource.pData;

    // 클립 평면을 클립 평면 상수 버퍼에 복사합니다.
    dataPtr2->clipPlane = clipPlane;

    // 버퍼의 락을 풉니다.
    deviceContext->Unmap(m_clipPlaneBuffer, 0);

    // 버텍스 셰이더에서 클립 평면 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 1;

    // 이제 업데이트된 값으로 버텍스 셰이더에 클립 평면 상수 버퍼를 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_clipPlaneBuffer);

    return true;
}


void ClipPlaneShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 버텍스 입력 레이아웃을 설정합니다.
    deviceContext->IASetInputLayout(m_layout);

    // 이 삼각형을 렌더링하는 데 사용할 버텍스 및 픽셀 셰이더를 설정합니다.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에 샘플러 상태를 설정합니다.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 지오메트리를 렌더링합니다.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}