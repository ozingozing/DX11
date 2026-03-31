////////////////////////////////////////////////////////////////////////////////
// 파일명: bitmapclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "bitmapclass.h"

// 클래스 생성자는 클래스 내부의 모든 private 포인터를 초기화합니다.
BitmapClass::BitmapClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
}


BitmapClass::BitmapClass(const BitmapClass& other)
{
}


BitmapClass::~BitmapClass()
{
}


bool BitmapClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, char* textureFilename, int renderX, int renderY)
{
    bool result;

    // Initialize 함수에서는 화면 크기와 이미지가 렌더링될 위치를 저장합니다.
    // 이 값들은 렌더링 시 정확한 정점 위치를 생성하는 데 필요합니다.

    // 화면 크기 저장.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // 비트맵이 렌더링될 위치 저장.
    m_renderX = renderX;
    m_renderY = renderY;

    // 버퍼를 생성하고, 이 비트맵 이미지에 사용할 텍스처도 함께 로드합니다.

    // 비트맵 사각형의 기하 정보를 담을 정점 버퍼와 인덱스 버퍼를 초기화합니다.
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // 이 비트맵에 사용할 텍스처를 로드합니다.
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

// Shutdown 함수는 비트맵 이미지에 사용된 텍스처와 정점/인덱스 버퍼를 해제합니다.
void BitmapClass::Shutdown()
{
    // 비트맵 텍스처 해제.
    ReleaseTexture();

    // 정점 버퍼와 인덱스 버퍼 해제.
    ShutdownBuffers();

    return;
}

// Render는 2D 이미지의 버퍼를 비디오 카드에 올립니다.
// UpdateBuffers 함수는 위치 정보와 함께 호출됩니다.
// 만약 이전 프레임과 비교해 위치가 변경되었다면, 동적 정점 버퍼 안의 정점 위치를 새 좌표로 갱신합니다.
// 위치가 변하지 않았다면 UpdateBuffers 호출을 건너뜁니다.
// 그 후 비트맵 텍스처를 설정하고, RenderBuffers 함수가 최종 렌더링을 위한 정점/인덱스를 준비합니다.
bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;

    // 비트맵 위치가 원래 위치에서 변경되었다면 버퍼를 갱신합니다.
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // 그래픽 파이프라인에 정점 버퍼와 인덱스 버퍼를 올려 드로잉 준비를 합니다.
    RenderBuffers(deviceContext);

    return true;
}

// GetIndexCount는 2D 이미지의 인덱스 개수를 반환합니다.
// 이 값은 대부분 항상 6입니다.
int BitmapClass::GetIndexCount()
{
    return m_indexCount;
}

// GetTexture 함수는 이 2D 이미지의 텍스처 리소스 포인터를 반환합니다.
// 셰이더가 이 함수를 호출하여 버퍼를 그릴 때 이미지를 사용할 수 있게 됩니다.
ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
    return m_Texture->GetTexture();
}

// InitializeBuffers는 2D 이미지를 그리는 데 사용할 정점 버퍼와 인덱스 버퍼를 생성하는 함수입니다.
bool BitmapClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // 이전 렌더링 위치를 먼저 -1로 초기화합니다.
    // 이 변수는 마지막으로 이미지가 그려진 위치를 저장하는 데 사용됩니다.
    // 이미지 위치가 이전 프레임과 같다면 동적 정점 버퍼를 수정하지 않아도 되므로 성능을 절약할 수 있습니다.

    // 이전 렌더링 위치를 -1로 초기화.
    m_prevPosX = -1;
    m_prevPosY = -1;

    // 정점 수는 6으로 설정합니다.
    // 사각형을 두 개의 삼각형으로 만들기 때문에 총 6개의 점이 필요합니다.
    // 인덱스 수도 동일합니다.

    // 정점 배열의 개수 설정.
    m_vertexCount = 6;

    // 인덱스 배열의 개수 설정.
    m_indexCount = m_vertexCount;

    // 정점 배열 생성.
    vertices = new VertexType[m_vertexCount];

    // 인덱스 배열 생성.
    indices = new unsigned long[m_indexCount];

    // 먼저 정점 배열을 0으로 초기화.
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // 인덱스 배열에 데이터 저장.
    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // 여기서 ModelClass와의 큰 차이점이 있습니다.
    // 이 정점 버퍼는 필요할 경우 매 프레임 수정할 수 있도록 동적 정점 버퍼로 생성합니다.
    // 동적으로 만들기 위해 Usage를 D3D11_USAGE_DYNAMIC으로,
    // CPUAccessFlags를 D3D11_CPU_ACCESS_WRITE로 설정합니다.

    // 동적 정점 버퍼 설명 설정.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 정점 데이터 포인터를 넘김.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 최종적으로 정점 버퍼 생성.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 인덱스 버퍼는 동적으로 만들 필요가 없습니다.
    // 정점 좌표는 바뀔 수 있어도, 6개의 인덱스는 항상 같은 6개의 정점을 가리키기 때문입니다.

    // 인덱스 버퍼 설명 설정.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 인덱스 데이터 포인터를 넘김.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼 생성.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 정점 버퍼와 인덱스 버퍼를 생성하고 데이터를 올린 후 배열 해제.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

// ShutdownBuffers는 정점 버퍼와 인덱스 버퍼를 해제합니다.
void BitmapClass::ShutdownBuffers()
{
    // 인덱스 버퍼 해제.
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // 정점 버퍼 해제.
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

// UpdateBuffers 함수는 필요할 경우 매 프레임 호출되어,
// 동적 정점 버퍼의 내용을 갱신하고 2D 비트맵 이미지를 화면에서 다시 배치합니다.
bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContent)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // 이 이미지를 렌더링할 위치가 바뀌었는지 확인합니다.
    // 바뀌지 않았다면 이번 프레임에서는 정점 버퍼를 수정할 필요가 없으므로 바로 종료합니다.
    // 이 검사는 많은 연산을 절약할 수 있습니다.

    // 현재 렌더링 위치가 이전과 같다면 정점 버퍼를 갱신하지 않음.
    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
    {
        return true;
    }

    // 렌더링 위치가 바뀌었다면 새 위치를 저장하고 정점 버퍼를 갱신합니다.

    // 렌더링 위치가 변경되었으므로 새 위치 저장.
    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;

    // 정점 배열 생성.
    vertices = new VertexType[m_vertexCount];

    // 이미지의 네 변 좌표를 계산해야 합니다.
    // 자세한 설명은 튜토리얼 상단의 도식을 참고하면 됩니다.

    // 비트맵 왼쪽 좌표 계산.
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;

    // 비트맵 오른쪽 좌표 계산.
    right = left + (float)m_bitmapWidth;

    // 비트맵 위쪽 좌표 계산.
    top = (float)(m_screenHeight / 2) - (float)m_renderY;

    // 비트맵 아래쪽 좌표 계산.
    bottom = top - (float)m_bitmapHeight;

    // 좌표 계산이 끝났으면 임시 정점 배열에 새 6개의 정점 데이터를 채웁니다.

    // 정점 배열에 데이터 입력.
    // 첫 번째 삼각형.
    vertices[0].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // 좌하단.
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // 두 번째 삼각형.
    vertices[3].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(right, top, 0.0f);  // 우상단.
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // 이제 Map과 memcpy를 사용해 정점 배열의 내용을 정점 버퍼에 복사합니다.

    // 정점 버퍼 잠금.
    result = deviceContent->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 버퍼 데이터에 접근할 포인터 획득.
    dataPtr = (VertexType*)mappedResource.pData;

    // 정점 데이터를 버퍼에 복사.
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // 정점 버퍼 잠금 해제.
    deviceContent->Unmap(m_vertexBuffer, 0);

    // 포인터 참조 해제.
    dataPtr = 0;

    // 더 이상 필요하지 않은 정점 배열 해제.
    delete[] vertices;
    vertices = 0;

    return true;
}

// RenderBuffers 함수는 정점 버퍼와 인덱스 버퍼를 GPU에 설정하여
// 셰이더가 그릴 수 있도록 준비합니다.
void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // 정점 버퍼의 stride와 offset 설정.
    stride = sizeof(VertexType);
    offset = 0;

    // 입력 조립기(Input Assembler)에 정점 버퍼를 활성화하여 렌더링 가능하게 설정.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // 입력 조립기에 인덱스 버퍼를 활성화하여 렌더링 가능하게 설정.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 이 정점 버퍼를 어떤 기본 도형으로 렌더링할지 설정.
    // 여기서는 삼각형 목록(Triangle List)입니다.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// 아래 함수는 2D 이미지를 그리는 데 사용할 텍스처를 로드합니다.
bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;

    // 텍스처 객체 생성 및 초기화.
    m_Texture = new TextureClass;

    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result)
    {
        return false;
    }

    // 이 비트맵이 픽셀 단위로 렌더링될 크기 저장.
    m_bitmapWidth = m_Texture->GetWidth();
    m_bitmapHeight = m_Texture->GetHeight();

    return true;
}

// ReleaseTexture 함수는 로드한 텍스처를 해제합니다.
void BitmapClass::ReleaseTexture()
{
    // 텍스처 객체 해제.
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}

// SetRenderLocation 함수는 2D 좌표를 사용하여
// 화면에서 비트맵 이미지가 렌더링될 위치를 변경할 수 있게 합니다.
void BitmapClass::SetRenderLocation(int x, int y)
{
    m_renderX = x;
    m_renderY = y;
    return;
}

void BitmapClass::SetRenderSize(int width, int height)
{
    m_bitmapWidth = width;
    m_bitmapHeight = height;
    // renderX/Y가 안 바뀌어도 크기가 바뀌면 업데이트해야 하므로
    // m_prevPosX를 강제로 무효화할 수도 있습니다.
    m_prevPosX = -1;
}