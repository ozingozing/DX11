////////////////////////////////////////////////////////////////////////////////
// 파일명: LightMapShaderClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "LightMapShaderClass.h"

LightMapShaderClass::LightMapShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;
}

LightMapShaderClass::LightMapShaderClass(const LightMapShaderClass& other)
{
}

LightMapShaderClass::~LightMapShaderClass()
{
}

bool LightMapShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // 이제 라이트 맵용 HLSL 셰이더 파일들(lightmap.vs, lightmap.ps)을 로드합니다.

    // 정점 셰이더 경로 설정
    error = wcscpy_s(vsFilename, 128, L"LightMap.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더 경로 설정
    error = wcscpy_s(psFilename, 128, L"LightMap.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 정점 및 픽셀 셰이더 초기화
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void LightMapShaderClass::Shutdown()
{
    // 셰이더 관련 인터페이스 및 객체 해제
    ShutdownShader();

    return;
}

bool LightMapShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2)
{
    bool result;

    // 렌더링에 필요한 셰이더 파라미터(행렬, 텍스처 등) 설정
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture1, texture2);
    if (!result)
    {
        return false;
    }

    // 준비된 버퍼를 사용하여 셰이더로 렌더링
    RenderShader(deviceContext, indexCount);

    return true;
}

bool LightMapShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 라이트 맵 정점 셰이더 컴파일 및 로드
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightMapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        else
        {
            MessageBox(hwnd, vsFilename, L"셰이더 파일을 찾을 수 없습니다.", MB_OK);
        }
        return false;
    }

    // 라이트 맵 픽셀 셰이더 컴파일 및 로드
    result = D3DCompileFromFile(psFilename, NULL, NULL, "LightMapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        else
        {
            MessageBox(hwnd, psFilename, L"셰이더 파일을 찾을 수 없습니다.", MB_OK);
        }
        return false;
    }

    // 버퍼로부터 정점 및 픽셀 셰이더 객체 생성
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result)) return false;

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result)) return false;

    // 정점 입력 레이아웃 설정
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

    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 입력 레이아웃 생성
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) return false;

    // 셰이더 버퍼 해제
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;
    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 정점 셰이더의 동적 행렬 상수 버퍼 설정
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result)) return false;

    // 텍스처 샘플러 상태 설정
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

    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result)) return false;

    return true;
}

void LightMapShaderClass::ShutdownShader()
{
    if (m_sampleState) { m_sampleState->Release(); m_sampleState = 0; }
    if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
    if (m_layout) { m_layout->Release(); m_layout = 0; }
    if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
    if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

void LightMapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    std::ofstream fout;

    compileErrors = (char*)(errorMessage->GetBufferPointer());
    bufferSize = errorMessage->GetBufferSize();

    fout.open("shader-error.txt");
    for (i = 0; i < bufferSize; i++) { fout << compileErrors[i]; }
    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"셰이더 컴파일 오류가 발생했습니다. shader-error.txt를 확인하세요.", shaderFilename, MB_OK);
}

bool LightMapShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // 행렬 전치 작업
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 상수 버퍼 매핑(Map) 및 행렬 복사
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) return false;

    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    deviceContext->Unmap(m_matrixBuffer, 0);

    // 정점 셰이더에 상수 버퍼 할당
    bufferNumber = 0;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // 텍스처 설정 방식은 이전 튜토리얼(멀티텍스처)과 동일합니다.
    // 다만 지금은 두 개의 색상 텍스처 대신, 하나는 '색상 텍스처', 다른 하나는 '라이트 맵'을 사용합니다.
    deviceContext->PSSetShaderResources(0, 1, &texture1);
    deviceContext->PSSetShaderResources(1, 1, &texture2);

    return true;
}

void LightMapShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    deviceContext->IASetInputLayout(m_layout);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에 샘플러 상태 설정
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 드로우 호출
    deviceContext->DrawIndexed(indexCount, 0, 0);
}