////////////////////////////////////////////////////////////////////////////////
// 파일명: fogshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "FogShaderClass.h"

FogShaderClass::FogShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;

    // 클래스 생성자에서 새로운 안개(fog) 버퍼를 초기화합니다.
    m_fogBuffer = 0;
}

FogShaderClass::FogShaderClass(const FogShaderClass& other)
{
}

FogShaderClass::~FogShaderClass()
{
}

bool FogShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // fog.vs 및 fog.ps HLSL 셰이더 파일을 로드합니다.

    // 버텍스 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(vsFilename, 128, L"fog.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(psFilename, 128, L"fog.hlsl");
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

void FogShaderClass::Shutdown()
{
    // 버텍스 및 픽셀 셰이더와 관련 객체들을 해제합니다.
    ShutdownShader();

    return;
}

// Render 함수는 이제 안개 시작 위치와 끝 위치 모두에 대해 float 값을 인자로 받습니다. 
// Render는 먼저 셰이더에서 이 파라미터를 설정한 다음 셰이더를 호출하여 드로잉을 시작합니다.
bool FogShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd)
{
    bool result;

    // 렌더링에 사용할 셰이더 파라미터를 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, fogStart, fogEnd);
    if (!result)
    {
        return false;
    }

    // 이제 준비된 버퍼를 셰이더와 함께 렌더링합니다.
    RenderShader(deviceContext, indexCount);

    return true;
}

bool FogShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC fogBufferDesc;

    // 이 함수가 사용할 포인터들을 null로 초기화합니다.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 안개 버텍스 셰이더를 로드합니다.

    // 버텍스 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "FogVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // 안개 픽셀 셰이더를 로드합니다.

    // 픽셀 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(psFilename, NULL, NULL, "FogPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // 안개 버퍼 설명을 설정한 다음 새로운 안개 동적 상수 버퍼를 생성합니다.

    // 버텍스 셰이더에 있는 동적 안개 상수 버퍼의 설명을 설정합니다.
    fogBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    fogBufferDesc.ByteWidth = sizeof(FogBufferType);
    fogBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    fogBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    fogBufferDesc.MiscFlags = 0;
    fogBufferDesc.StructureByteStride = 0;

    // 이 클래스 내에서 버텍스 셰이더 안개 상수 버퍼에 접근할 수 있도록 안개 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&fogBufferDesc, NULL, &m_fogBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void FogShaderClass::ShutdownShader()
{
    // ShutdownShader 함수에서 새로운 안개 버퍼를 해제합니다.

    // 안개 상수 버퍼를 해제합니다.
    if (m_fogBuffer)
    {
        m_fogBuffer->Release();
        m_fogBuffer = 0;
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

void FogShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

bool FogShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    FogBufferType* dataPtr2;

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

    // 여기서는 버퍼를 락하고, 데이터를 설정한 다음 다시 언락하여 상수 버퍼에 안개 시작 및 끝 위치를 설정합니다. 
    // 이는 버텍스 셰이더의 두 번째 버퍼이므로 bufferNumber를 0에서 1로 변경합니다.

    // 안개 상수 버퍼에 쓸 수 있도록 락을 겁니다.
    result = deviceContext->Map(m_fogBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내 데이터에 대한 포인터를 얻습니다.
    dataPtr2 = (FogBufferType*)mappedResource.pData;

    // 안개 정보를 안개 상수 버퍼에 복사합니다.
    dataPtr2->fogStart = fogStart;
    dataPtr2->fogEnd = fogEnd;

    // 상수 버퍼의 락을 풉니다.
    deviceContext->Unmap(m_fogBuffer, 0);

    // 버텍스 셰이더에서 안개 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 1;

    // 이제 업데이트된 값으로 버텍스 셰이더에 안개 버퍼를 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_fogBuffer);

    return true;
}

void FogShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 버텍스 입력 레이아웃을 설정합니다.
    deviceContext->IASetInputLayout(m_layout);

    // 이 삼각형을 렌더링하는 데 사용할 버텍스 및 픽셀 셰이더를 설정합니다.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에 샘플러 상태를 설정합니다.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 삼각형을 렌더링합니다.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}