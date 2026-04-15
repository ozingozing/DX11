////////////////////////////////////////////////////////////////////////////////
// 파일명: fontshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "FontShaderClass.h"

FontShaderClass::FontShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;

    // 픽셀 셰이더에서 사용할 색상용 상수 버퍼 초기화
    m_pixelBuffer = 0;
}

FontShaderClass::FontShaderClass(const FontShaderClass& other) {}
FontShaderClass::~FontShaderClass() {}

bool FontShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // 폰트용 정점/픽셀 셰이더 파일 경로 설정
    error = wcscpy_s(vsFilename, 128, L"font.hlsl");
    if (error != 0) return false;

    error = wcscpy_s(psFilename, 128, L"font.hlsl");
    if (error != 0) return false;

    // 실제 셰이더 객체 및 버퍼 생성 함수 호출
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
    if (!result) return false;

    return true;
}

void FontShaderClass::Shutdown()
{
    // 셰이더와 관련된 모든 리소스 해제
    ShutdownShader();
}

bool FontShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
    bool result;

    // 1. 렌더링에 필요한 파라미터(행렬, 텍스처, 폰트 색상)를 셰이더에 전달
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);
    if (!result) return false;

    // 2. 준비된 버퍼를 사용하여 실제 그리기 수행
    RenderShader(deviceContext, indexCount);

    return true;
}

bool FontShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage = 0;
    ID3D10Blob* vertexShaderBuffer = 0;
    ID3D10Blob* pixelShaderBuffer = 0;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC pixelBufferDesc;

    // [정점 셰이더 컴파일] 진입점: FontVertexShader
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        else MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // [픽셀 셰이더 컴파일] 진입점: FontPixelShader
    result = D3DCompileFromFile(psFilename, NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        else MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        return false;
    }

    // 셰이더 객체 생성
    device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);

    // 정점 입력 레이아웃 설정 (Position, TexCoord)
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

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
    device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);

    vertexShaderBuffer->Release();
    pixelShaderBuffer->Release();

    // 행렬용 상수 버퍼 설정 (Dynamic 사용 - 매 프레임 갱신)
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

    // 샘플러 상태 설정 (텍스처 필터링 방식 결정)
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0; samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0; samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    device->CreateSamplerState(&samplerDesc, &m_sampleState);

    // [중요] 픽셀 컬러용 상수 버퍼 설정
    pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
    pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pixelBufferDesc.MiscFlags = 0;
    pixelBufferDesc.StructureByteStride = 0;

    device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);

    return true;
}

void FontShaderClass::ShutdownShader()
{
    // 리소스 할당의 역순으로 해제
    if (m_pixelBuffer) { m_pixelBuffer->Release(); m_pixelBuffer = 0; }
    if (m_sampleState) { m_sampleState->Release(); m_sampleState = 0; }
    if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
    if (m_layout) { m_layout->Release(); m_layout = 0; }
    if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
    if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

// 셰이더 컴파일 에러 발생 시 파일로 기록하는 함수
void FontShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    ofstream fout;

    compileErrors = (char*)(errorMessage->GetBufferPointer());
    bufferSize = errorMessage->GetBufferSize();
    fout.open("shader-error.txt");

    for (i = 0; i < bufferSize; i++) fout << compileErrors[i];

    fout.close();
    errorMessage->Release();
    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt.", shaderFilename, MB_OK);
}

bool FontShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    PixelBufferType* dataPtr2;

    // GPU 연산을 위해 행렬을 전치(Transpose) 처리
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 1. 행렬 상수 버퍼 잠금 및 데이터 복사 (VS용)
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) return false;
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;
    deviceContext->Unmap(m_matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

    // 2. 텍스처 리소스 설정 (PS용)
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // 3. 픽셀 컬러 상수 버퍼 잠금 및 데이터 복사 (PS용)
    // 이 작업을 통해 CPU에서 지정한 글자 색상을 셰이더로 보냅니다.
    result = deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) return false;
    dataPtr2 = (PixelBufferType*)mappedResource.pData;
    dataPtr2->pixelColor = pixelColor;
    deviceContext->Unmap(m_pixelBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &m_pixelBuffer);

    return true;
}

void FontShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 입력 레이아웃 및 셰이더 설정
    deviceContext->IASetInputLayout(m_layout);
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 최종 드로우 콜 (인덱스 버퍼를 기반으로 렌더링)
    deviceContext->DrawIndexed(indexCount, 0, 0);
}