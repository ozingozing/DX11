////////////////////////////////////////////////////////////////////////////////
// 파일명: normalmapshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "normalmapshaderclass.h"

NormalMapShaderClass::NormalMapShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;
    m_lightBuffer = 0;
}

NormalMapShaderClass::NormalMapShaderClass(const NormalMapShaderClass& other)
{
}

NormalMapShaderClass::~NormalMapShaderClass()
{
}

bool NormalMapShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // Initialize 함수는 노멀 맵 HLSL 파일들을 로드하기 위해 호출됩니다.

    // 정점 셰이더 파일 경로 설정
    error = wcscpy_s(vsFilename, 128, L"Normalmap.hlsl");
    if (error != 0) return false;

    // 픽셀 셰이더 파일 경로 설정
    error = wcscpy_s(psFilename, 128, L"Normalmap.hlsl");
    if (error != 0) return false;

    // 정점 및 픽셀 셰이더 초기화
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
    if (!result) return false;

    return true;
}

void NormalMapShaderClass::Shutdown()
{
    // 셰이더 및 관련 객체들을 해제합니다.
    ShutdownShader();
}

// Render 함수는 행렬, 컬러 및 노멀 텍스처, 그리고 광원의 방향과 색상을 입력으로 받습니다.
bool NormalMapShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
    bool result;

    // 렌더링에 사용할 셰이더 파라미터들을 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture1, texture2, lightDirection, diffuseColor);
    if (!result) return false;

    // 준비된 버퍼를 셰이더로 렌더링합니다.
    RenderShader(deviceContext, indexCount);

    return true;
}

bool NormalMapShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;

    // 탄젠트(Tangent)와 바이노멀(Binormal)을 수용하기 위해 폴리곤 레이아웃을 5개 요소로 설정합니다.
    D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC lightBufferDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 노멀 맵 정점 셰이더 컴파일
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "NormalMapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        else MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // 노멀 맵 픽셀 셰이더 컴파일
    result = D3DCompileFromFile(psFilename, NULL, NULL, "NormalMapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        else MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // 버퍼로부터 셰이더 생성
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result)) return false;

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result)) return false;

    // 정점 입력 레이아웃 설정 (Position, Texcoord, Normal)
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

    // 레이아웃에 TANGENT와 BINORMAL 요소를 추가합니다.
    // 시맨틱 이름을 제외하고는 NORMAL 요소 설정과 동일합니다.
    polygonLayout[3].SemanticName = "TANGENT";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    polygonLayout[4].SemanticName = "BINORMAL";
    polygonLayout[4].SemanticIndex = 0;
    polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[4].InputSlot = 0;
    polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[4].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 입력 레이아웃 생성
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) return false;

    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;
    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 행렬 상수 버퍼 설명 설정
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result)) return false;

    // 샘플러 상태 설정
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0; samplerDesc.BorderColor[1] = 0; samplerDesc.BorderColor[2] = 0; samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result)) return false;

    // 광원 정보를 위한 상수 버퍼를 설정합니다.
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if (FAILED(result)) return false;

    return true;
}

void NormalMapShaderClass::ShutdownShader()
{
    if (m_lightBuffer) { m_lightBuffer->Release(); m_lightBuffer = 0; }
    if (m_sampleState) { m_sampleState->Release(); m_sampleState = 0; }
    if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
    if (m_layout) { m_layout->Release(); m_layout = 0; }
    if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
    if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

void NormalMapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors = (char*)(errorMessage->GetBufferPointer());
    unsigned long long bufferSize = errorMessage->GetBufferSize();
    std::ofstream fout;

    fout.open("shader-error.txt");
    for (unsigned long long i = 0; i < bufferSize; i++) fout << compileErrors[i];
    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool NormalMapShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    LightBufferType* dataPtr2;

    // 행렬 전치(Transpose)
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 행렬 상수 버퍼 잠금 및 쓰기
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) return false;

    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    deviceContext->Unmap(m_matrixBuffer, 0);

    // 정점 셰이더에 행렬 버퍼 설정
    bufferNumber = 0;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // 여기서 픽셀 셰이더에 컬러 텍스처와 노멀 텍스처를 설정합니다.
    deviceContext->PSSetShaderResources(0, 1, &texture1);
    deviceContext->PSSetShaderResources(1, 1, &texture2);

    // 픽셀 셰이더의 광원 상수 버퍼에 확산광 색상과 광원 방향을 설정합니다.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) return false;

    dataPtr2 = (LightBufferType*)mappedResource.pData;
    dataPtr2->diffuseColor = diffuseColor;
    dataPtr2->lightDirection = lightDirection;
    dataPtr2->padding = 0.0f;

    deviceContext->Unmap(m_lightBuffer, 0);

    // 픽셀 셰이더에 광원 상수 버퍼 설정
    bufferNumber = 1;
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

    return true;
}

void NormalMapShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    deviceContext->IASetInputLayout(m_layout);
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    deviceContext->DrawIndexed(indexCount, 0, 0);
}