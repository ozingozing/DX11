////////////////////////////////////////////////////////////////////////////////
// 파일명: FireShaderClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "FireShaderClass.h"

// 클래스 생성자에서 클래스의 모든 private 포인터들을 null로 초기화합니다.
FireShaderClass::FireShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_noiseBuffer = 0;
    m_distortionBuffer = 0;
    m_sampleStateWrap = 0;
    m_sampleStateClamp = 0;
}


FireShaderClass::FireShaderClass(const FireShaderClass& other)
{
}


FireShaderClass::~FireShaderClass()
{
}


bool FireShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    wchar_t vsFilename[128], psFilename[128];
    int error;
    bool result;

    // 여기서 fire.vs 및 fire.ps HLSL 셰이더 파일을 로드합니다.

    // 버텍스 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(vsFilename, 128, L"Fire.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더의 파일명을 설정합니다.
    error = wcscpy_s(psFilename, 128, L"Fire.hlsl");
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


void FireShaderClass::Shutdown()
{
    // 버텍스 및 픽셀 셰이더와 관련 객체들을 해제합니다.
    ShutdownShader();

    return;
}

// Render 함수는 화염의 모습을 렌더링하고 조절하는 데 사용되는 수많은 입력 변수들을 받습니다.
// 이 값들은 먼저 SetShaderParameters 함수를 사용하여 셰이더에 설정됩니다.
// 모든 값이 설정되면 나중에 호출되는 RenderShader 함수에 의해 셰이더가 렌더링에 사용됩니다.
bool FireShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* fireTexture, ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
    XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2, XMFLOAT2 distortion3,
    float distortionScale, float distortionBias)
{
    bool result;


    // 렌더링에 사용할 셰이더 파라미터를 설정합니다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, fireTexture, noiseTexture, alphaTexture, frameTime,
        scrollSpeeds, scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);
    if (!result)
    {
        return false;
    }

    // 이제 준비된 버퍼를 셰이더와 함께 렌더링합니다.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool FireShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC noiseBufferDesc;
    D3D11_BUFFER_DESC distortionBufferDesc;
    D3D11_SAMPLER_DESC samplerDescWrap;
    D3D11_SAMPLER_DESC samplerDescClamp;


    // 이 함수가 사용할 포인터들을 null로 초기화합니다.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 화염 버텍스 셰이더를 로드합니다.

    // 버텍스 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "FireVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // 화염 픽셀 셰이더를 로드합니다.

    // 픽셀 셰이더 코드를 컴파일합니다.
    result = D3DCompileFromFile(psFilename, NULL, NULL, "FirePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // 새로운 노이즈 버퍼를 설정하고 생성합니다.

    // 버텍스 셰이더에 있는 동적 노이즈 상수 버퍼의 설명을 설정합니다.
    noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    noiseBufferDesc.ByteWidth = sizeof(NoiseBufferType);
    noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    noiseBufferDesc.MiscFlags = 0;
    noiseBufferDesc.StructureByteStride = 0;

    // 이 클래스 내에서 버텍스 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&noiseBufferDesc, NULL, &m_noiseBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 새로운 왜곡 버퍼를 설정하고 생성합니다.

    // 픽셀 셰이더에 있는 동적 왜곡 상수 버퍼의 설명을 설정합니다.
    distortionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    distortionBufferDesc.ByteWidth = sizeof(DistortionBufferType);
    distortionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    distortionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    distortionBufferDesc.MiscFlags = 0;
    distortionBufferDesc.StructureByteStride = 0;

    // 이 클래스 내에서 픽셀 셰이더 상수 버퍼에 접근할 수 있도록 상수 버퍼 포인터를 생성합니다.
    result = device->CreateBuffer(&distortionBufferDesc, NULL, &m_distortionBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Wrap 텍스처 샘플러 상태 설명을 생성합니다.
    samplerDescWrap.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDescWrap.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescWrap.MipLODBias = 0.0f;
    samplerDescWrap.MaxAnisotropy = 1;
    samplerDescWrap.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDescWrap.BorderColor[0] = 0;
    samplerDescWrap.BorderColor[1] = 0;
    samplerDescWrap.BorderColor[2] = 0;
    samplerDescWrap.BorderColor[3] = 0;
    samplerDescWrap.MinLOD = 0;
    samplerDescWrap.MaxLOD = D3D11_FLOAT32_MAX;

    // 텍스처 샘플러 상태를 생성합니다.
    result = device->CreateSamplerState(&samplerDescWrap, &m_sampleStateWrap);
    if (FAILED(result))
    {
        return false;
    }

    // WRAP 대신 CLAMP를 사용하는 새로운 샘플러 상태를 설정하고 생성합니다.

    // Clamp 텍스처 샘플러 상태 설명을 생성합니다.
    samplerDescClamp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDescClamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescClamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescClamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescClamp.MipLODBias = 0.0f;
    samplerDescClamp.MaxAnisotropy = 1;
    samplerDescClamp.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDescClamp.BorderColor[0] = 0;
    samplerDescClamp.BorderColor[1] = 0;
    samplerDescClamp.BorderColor[2] = 0;
    samplerDescClamp.BorderColor[3] = 0;
    samplerDescClamp.MinLOD = 0;
    samplerDescClamp.MaxLOD = D3D11_FLOAT32_MAX;

    // 텍스처 샘플러 상태를 생성합니다.
    result = device->CreateSamplerState(&samplerDescClamp, &m_sampleStateClamp);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void FireShaderClass::ShutdownShader()
{
    // ShutdownShader 함수는 화염 셰이더 내부 값들에 접근하는 데 사용되었던 모든 포인터들을 해제합니다.

    // 샘플러 상태들을 해제합니다.
    if (m_sampleStateClamp)
    {
        m_sampleStateClamp->Release();
        m_sampleStateClamp = 0;
    }

    if (m_sampleStateWrap)
    {
        m_sampleStateWrap->Release();
        m_sampleStateWrap = 0;
    }

    // 왜곡 상수 버퍼를 해제합니다.
    if (m_distortionBuffer)
    {
        m_distortionBuffer->Release();
        m_distortionBuffer = 0;
    }

    // 노이즈 상수 버퍼를 해제합니다.
    if (m_noiseBuffer)
    {
        m_noiseBuffer->Release();
        m_noiseBuffer = 0;
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


void FireShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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


bool FireShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* fireTexture, ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture,
    float frameTime, XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2, XMFLOAT2 distortion3,
    float distortionScale, float distortionBias)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    NoiseBufferType* dataPtr2;
    DistortionBufferType* dataPtr3;
    unsigned int bufferNumber;


    // 셰이더에 전달할 수 있도록 매트릭스들을 전치(Transpose)합니다.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 평소처럼 버텍스 셰이더에 매트릭스 버퍼를 설정합니다.

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

    // 버텍스 셰이더에 새로운 노이즈 버퍼를 설정합니다.

    // 노이즈 상수 버퍼에 쓸 수 있도록 락을 겁니다.
    result = deviceContext->Map(m_noiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 노이즈 상수 버퍼 내 데이터에 대한 포인터를 얻습니다.
    dataPtr2 = (NoiseBufferType*)mappedResource.pData;

    // 데이터를 노이즈 상수 버퍼에 복사합니다.
    dataPtr2->frameTime = frameTime;
    dataPtr2->scrollSpeeds = scrollSpeeds;
    dataPtr2->scales = scales;
    dataPtr2->padding = 0.0f;

    // 노이즈 상수 버퍼의 락을 풉니다.
    deviceContext->Unmap(m_noiseBuffer, 0);

    // 버텍스 셰이더에서 노이즈 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 1;

    // 이제 업데이트된 값으로 버텍스 셰이더에 노이즈 상수 버퍼를 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_noiseBuffer);

    // 픽셀 셰이더에 3개의 텍스처를 설정합니다.

    // 픽셀 셰이더에 3개의 셰이더 텍스처 리소스를 설정합니다.
    deviceContext->PSSetShaderResources(0, 1, &fireTexture);
    deviceContext->PSSetShaderResources(1, 1, &noiseTexture);
    deviceContext->PSSetShaderResources(2, 1, &alphaTexture);

    // 픽셀 셰이더에 새로운 왜곡 버퍼를 설정합니다.

    // 왜곡 상수 버퍼에 쓸 수 있도록 락을 겁니다.
    result = deviceContext->Map(m_distortionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 왜곡 상수 버퍼 내 데이터에 대한 포인터를 얻습니다.
    dataPtr3 = (DistortionBufferType*)mappedResource.pData;

    // 데이터를 왜곡 상수 버퍼에 복사합니다.
    dataPtr3->distortion1 = distortion1;
    dataPtr3->distortion2 = distortion2;
    dataPtr3->distortion3 = distortion3;
    dataPtr3->distortionScale = distortionScale;
    dataPtr3->distortionBias = distortionBias;

    // 왜곡 상수 버퍼의 락을 풉니다.
    deviceContext->Unmap(m_distortionBuffer, 0);

    // 픽셀 셰이더에서 왜곡 상수 버퍼의 위치를 설정합니다.
    bufferNumber = 0;

    // 이제 업데이트된 값으로 픽셀 셰이더에 왜곡 상수 버퍼를 설정합니다.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_distortionBuffer);

    return true;
}


void FireShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 버텍스 입력 레이아웃을 설정합니다.
    deviceContext->IASetInputLayout(m_layout);

    // 지오메트리를 렌더링하는 데 사용할 버텍스 및 픽셀 셰이더를 설정합니다.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에 샘플러 상태들을 설정합니다.
    deviceContext->PSSetSamplers(0, 1, &m_sampleStateWrap);

    // 픽셀 셰이더에 CLAMP를 사용하는 새로운 샘플러 상태를 설정합니다.
    deviceContext->PSSetSamplers(1, 1, &m_sampleStateClamp);

    // 지오메트리를 렌더링합니다.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}