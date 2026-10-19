////////////////////////////////////////////////////////////////////////////////
// 파일명: DisplayPlaneClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "DisplayPlaneClass.h"

DisplayPlaneClass::DisplayPlaneClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
}

DisplayPlaneClass::DisplayPlaneClass(const DisplayPlaneClass& other)
{
}

DisplayPlaneClass::~DisplayPlaneClass()
{
}

// 너비(width)와 높이(height) 입력값은 3D 좌표 기준입니다. 예를 들어, 이 튜토리얼에서는 너비와 높이가 모두 1.0f입니다.
// DisplayPlane 객체를 생성할 때 사용되는 가로세로 비율(aspect ratio)이 렌더 텍스처를 생성할 때 사용된 치수와 일치하는지 확인하세요.
// 그렇지 않으면 텍스처가 잘못 늘어나게 됩니다.
bool DisplayPlaneClass::Initialize(ID3D11Device* device, float width, float height)
{
    bool result;

    // 버튼의 지오메트리를 담고 있는 버텍스 및 인덱스 버퍼를 초기화합니다.
    result = InitializeBuffers(device, width, height);
    if (!result)
    {
        return false;
    }

    return true;
}

void DisplayPlaneClass::Shutdown()
{
    // 버텍스 및 인덱스 버퍼를 해제합니다.
    ShutdownBuffers();

    return;
}

void DisplayPlaneClass::Render(ID3D11DeviceContext* deviceContext)
{
    // 드로잉을 준비하기 위해 버텍스 및 인덱스 버퍼를 그래픽 파이프라인에 배치합니다.
    RenderBuffers(deviceContext);

    return;
}

int DisplayPlaneClass::GetIndexCount()
{
    return m_indexCount;
}

// BitmapClass와 유사하게, 렌더링될 모델로서 두 개의 삼각형으로 이루어진 객체를 생성합니다.
bool DisplayPlaneClass::InitializeBuffers(ID3D11Device* device, float width, float height)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // 버텍스 배열의 버텍스 개수를 설정합니다.
    m_vertexCount = 6;

    // 인덱스 배열의 인덱스 개수를 설정합니다.
    m_indexCount = m_vertexCount;

    // 버텍스 배열을 생성합니다.
    vertices = new VertexType[m_vertexCount];

    // 인덱스 배열을 생성합니다.
    indices = new unsigned long[m_indexCount];

    // 버텍스 배열에 데이터를 로드합니다.
    // 첫 번째 삼각형.
    vertices[0].position = XMFLOAT3(-width, height, 0.0f);  // 좌측 상단.
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(width, -height, 0.0f);  // 우측 하단.
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(-width, -height, 0.0f);  // 좌측 하단.
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // 두 번째 삼각형.
    vertices[3].position = XMFLOAT3(-width, height, 0.0f);  // 좌측 상단.
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(width, height, 0.0f);  // 우측 상단.
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(width, -height, 0.0f);  // 우측 하단.
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // 인덱스 배열에 데이터를 로드합니다.
    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // 버텍스 버퍼의 설명을 설정합니다.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 버텍스 데이터의 포인터를 지정합니다.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 최종적으로 버텍스 버퍼를 생성합니다.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 인덱스 버퍼의 설명을 설정합니다.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 인덱스 데이터의 포인터를 지정합니다.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼를 생성합니다.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 버텍스 및 인덱스 버퍼가 생성되고 데이터가 로드되었으므로 배열들을 해제합니다.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void DisplayPlaneClass::ShutdownBuffers()
{
    // 인덱스 버퍼를 해제합니다.
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // 버텍스 버퍼를 해제합니다.
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

void DisplayPlaneClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // 버텍스 버퍼의 스트라이드와 오프셋을 설정합니다.
    stride = sizeof(VertexType);
    offset = 0;

    // 렌더링될 수 있도록 입력 Assembler에 버텍스 버퍼를 활성 상태로 설정합니다.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // 렌더링될 수 있도록 입력 Assembler에 인덱스 버퍼를 활성 상태로 설정합니다.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 이 버텍스 버퍼로부터 렌더링해야 할 프리미티브 유형(여기서는 삼각형)을 설정합니다.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}