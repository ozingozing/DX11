#include "SpecMapShaderClass.h"

SpecMapShaderClass::SpecMapShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;

    // 조명 버퍼와 카메라 버퍼 포인터를 null로 초기화한다.
    m_lightBuffer = 0;
    m_cameraBuffer = 0;
}


SpecMapShaderClass::SpecMapShaderClass(const SpecMapShaderClass& other)
{
}


SpecMapShaderClass::~SpecMapShaderClass()
{
}


bool SpecMapShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // 사용할 정점 셰이더와 픽셀 셰이더 파일을 지정한다.

    // 정점 셰이더 파일 이름을 설정한다.
    error = wcscpy_s(vsFilename, 128, L"Specmap.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 픽셀 셰이더 파일 이름을 설정한다.
    error = wcscpy_s(psFilename, 128, L"Specmap.hlsl");
    if (error != 0)
    {
        return false;
    }

    // 정점 셰이더와 픽셀 셰이더를 초기화한다.
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
    if (!result)
    {
        return false;
    }

    return true;
}


void SpecMapShaderClass::Shutdown()
{
    // 정점 셰이더, 픽셀 셰이더, 관련 리소스를 해제한다.
    ShutdownShader();

    return;
}


// Render 함수는 스페큘러 조명 계산에 필요한 카메라 위치,
// 스페큘러 색상, 스페큘러 강도 값을 함께 전달받는다.
bool SpecMapShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3,
    XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    bool result;

    // 렌더링에 사용할 셰이더 매개변수들을 설정한다.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture1, texture2, texture3, lightDirection, diffuseColor,
        cameraPosition, specularColor, specularPower);
    if (!result)
    {
        return false;
    }

    // 설정된 셰이더와 버퍼를 사용하여 렌더링을 수행한다.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool SpecMapShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC lightBufferDesc;
    D3D11_BUFFER_DESC cameraBufferDesc;

    // 이 함수에서 사용할 포인터들을 null로 초기화한다.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 스페큘러 맵용 정점 셰이더를 컴파일한다.

    // 정점 셰이더 코드를 컴파일한다.
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "SpecMapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패했을 경우, 오류 메시지가 있으면 파일로 출력한다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // 오류 메시지가 없다면 셰이더 파일 자체를 찾지 못한 것이다.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // 스페큘러 맵용 픽셀 셰이더를 컴파일한다.

    // 픽셀 셰이더 코드를 컴파일한다.
    result = D3DCompileFromFile(psFilename, NULL, NULL, "SpecMapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 셰이더 컴파일에 실패했을 경우, 오류 메시지가 있으면 파일로 출력한다.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // 오류 메시지가 없다면 셰이더 파일 자체를 찾지 못한 것이다.
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // 컴파일된 버퍼를 사용하여 정점 셰이더를 생성한다.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    // 컴파일된 버퍼를 사용하여 픽셀 셰이더를 생성한다.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // 정점 입력 레이아웃을 설정한다.

    // 정점 위치 정보
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    // 텍스처 좌표 정보
    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // 노멀 벡터 정보
    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // 탄젠트 벡터 정보
    polygonLayout[3].SemanticName = "TANGENT";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    // 바이노멀 벡터 정보
    polygonLayout[4].SemanticName = "BINORMAL";
    polygonLayout[4].SemanticIndex = 0;
    polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[4].InputSlot = 0;
    polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[4].InstanceDataStepRate = 0;

    // 입력 레이아웃 요소 개수를 계산한다.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 정점 입력 레이아웃을 생성한다.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // 셰이더 생성이 끝났으므로 정점 셰이더 버퍼와 픽셀 셰이더 버퍼를 해제한다.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 정점 셰이더에서 사용할 동적 행렬 상수 버퍼 정보를 설정한다.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // 이 클래스에서 접근할 수 있도록 행렬 상수 버퍼를 생성한다.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 텍스처 샘플러 상태 정보를 설정한다.
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

    // 텍스처 샘플러 상태를 생성한다.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result))
    {
        return false;
    }

    // 조명 버퍼와 카메라 버퍼를 설정한다.

    // 픽셀 셰이더에서 사용할 동적 조명 상수 버퍼 정보를 설정한다.
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // 이 클래스에서 접근할 수 있도록 조명 상수 버퍼를 생성한다.
    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 정점 셰이더에서 사용할 동적 카메라 상수 버퍼 정보를 설정한다.
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;

    // 이 클래스에서 접근할 수 있도록 카메라 상수 버퍼를 생성한다.
    result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}


void SpecMapShaderClass::ShutdownShader()
{
    // 스페큘러 조명 계산에 사용한 조명 버퍼와 카메라 버퍼를 포함하여
    // 셰이더 관련 리소스를 모두 해제한다.

    // 카메라 상수 버퍼를 해제한다.
    if (m_cameraBuffer)
    {
        m_cameraBuffer->Release();
        m_cameraBuffer = 0;
    }

    // 조명 상수 버퍼를 해제한다.
    if (m_lightBuffer)
    {
        m_lightBuffer->Release();
        m_lightBuffer = 0;
    }

    // 샘플러 상태를 해제한다.
    if (m_sampleState)
    {
        m_sampleState->Release();
        m_sampleState = 0;
    }

    // 행렬 상수 버퍼를 해제한다.
    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    // 입력 레이아웃을 해제한다.
    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }

    // 픽셀 셰이더를 해제한다.
    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    // 정점 셰이더를 해제한다.
    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }

    return;
}


void SpecMapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    ofstream fout;

    // 오류 메시지 텍스트 버퍼의 포인터를 가져온다.
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // 오류 메시지의 길이를 가져온다.
    bufferSize = errorMessage->GetBufferSize();

    // 오류 메시지를 저장할 파일을 연다.
    fout.open("shader-error.txt");

    // 오류 메시지를 파일에 기록한다.
    for (i = 0; i < bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // 파일을 닫는다.
    fout.close();

    // 오류 메시지 객체를 해제한다.
    errorMessage->Release();
    errorMessage = 0;

    // 사용자에게 셰이더 컴파일 오류 파일을 확인하라는 메시지를 출력한다.
    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}


// SetShaderParameters 함수는 세 개의 텍스처와 카메라 위치,
// 스페큘러 색상, 스페큘러 강도 값을 셰이더에 전달한다.
bool SpecMapShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3,
    XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    LightBufferType* dataPtr2;
    CameraBufferType* dataPtr3;

    // HLSL 셰이더에서 사용할 수 있도록 행렬을 전치한다.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 행렬 상수 버퍼에 데이터를 쓰기 위해 잠근다.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내부 데이터에 접근할 포인터를 가져온다.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // 월드, 뷰, 투영 행렬을 상수 버퍼에 복사한다.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // 행렬 상수 버퍼의 잠금을 해제한다.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // 정점 셰이더에서 행렬 상수 버퍼가 들어갈 슬롯 번호를 설정한다.
    bufferNumber = 0;

    // 갱신된 행렬 상수 버퍼를 정점 셰이더에 설정한다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // 기본 색상 텍스처, 노멀 맵, 스페큘러 맵을 픽셀 셰이더에 설정한다.

    // 픽셀 셰이더에 텍스처 리소스를 설정한다.
    deviceContext->PSSetShaderResources(0, 1, &texture1);
    deviceContext->PSSetShaderResources(1, 1, &texture2);
    deviceContext->PSSetShaderResources(2, 1, &texture3);

    // 조명 상수 버퍼를 설정한다.

    // 조명 상수 버퍼에 데이터를 쓰기 위해 잠근다.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내부 데이터에 접근할 포인터를 가져온다.
    dataPtr2 = (LightBufferType*)mappedResource.pData;

    // 조명 관련 변수들을 상수 버퍼에 복사한다.
    dataPtr2->diffuseColor = diffuseColor;
    dataPtr2->lightDirection = lightDirection;
    dataPtr2->specularColor = specularColor;
    dataPtr2->specularPower = specularPower;

    // 조명 상수 버퍼의 잠금을 해제한다.
    deviceContext->Unmap(m_lightBuffer, 0);

    // 픽셀 셰이더에서 조명 상수 버퍼가 들어갈 슬롯 번호를 설정한다.
    bufferNumber = 0;

    // 갱신된 조명 상수 버퍼를 픽셀 셰이더에 설정한다.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

    // 카메라 상수 버퍼를 설정한다.

    // 카메라 상수 버퍼에 데이터를 쓰기 위해 잠근다.
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 상수 버퍼 내부 데이터에 접근할 포인터를 가져온다.
    dataPtr3 = (CameraBufferType*)mappedResource.pData;

    // 카메라 위치를 상수 버퍼에 복사한다.
    dataPtr3->cameraPosition = cameraPosition;

    // 카메라 상수 버퍼의 잠금을 해제한다.
    deviceContext->Unmap(m_cameraBuffer, 0);

    // 정점 셰이더에서 카메라 상수 버퍼가 들어갈 슬롯 번호를 설정한다.
    // 행렬 버퍼가 0번 슬롯을 사용하므로 카메라 버퍼는 1번 슬롯을 사용한다.
    bufferNumber = 1;

    // 갱신된 카메라 상수 버퍼를 정점 셰이더에 설정한다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

    return true;
}


void SpecMapShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 정점 입력 레이아웃을 설정한다.
    deviceContext->IASetInputLayout(m_layout);

    // 렌더링에 사용할 정점 셰이더와 픽셀 셰이더를 설정한다.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 픽셀 셰이더에 샘플러 상태를 설정한다.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 인덱스 버퍼를 기준으로 모델을 렌더링한다.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}