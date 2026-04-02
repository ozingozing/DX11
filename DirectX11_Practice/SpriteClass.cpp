////////////////////////////////////////////////////////////////////////////////
// 파일명: spriteclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Spriteclass.h"

SpriteClass::SpriteClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Textures = 0;
}

SpriteClass::SpriteClass(const SpriteClass& other)
{
}

SpriteClass::~SpriteClass()
{
}

// Initialize 함수는 기본적으로 BitmapClass와 유사하지만,
// 스프라이트 설정 정보가 담긴 텍스트 파일 경로(spriteFilename)를 추가로 받습니다.
bool SpriteClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, char* spriteFilename, int renderX, int renderY)
{
    bool result;

    // 화면 크기 저장
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // 스프라이트가 렌더링될 위치 저장
    m_renderX = renderX;
    m_renderY = renderY;

    // 애니메이션 프레임 시간 초기화 (순환 속도 제어용)
    m_frameTime = 0;

    // 스프라이트의 기하학적 구조(사각형)를 위한 정점 및 인덱스 버퍼 초기화
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // 외부 설정 파일을 참조하여 스프라이트용 텍스처 배열 로드
    result = LoadTextures(device, deviceContext, spriteFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void SpriteClass::Shutdown()
{
    // 사용된 텍스처 배열 해제
    ReleaseTextures();

    // 정점 및 인덱스 버퍼 해제
    ShutdownBuffers();

    return;
}

bool SpriteClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;

    // 스프라이트의 위치가 변경되었다면 동적 정점 버퍼를 갱신합니다.
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // 그리기 준비를 위해 버퍼를 그래픽 파이프라인에 올립니다.
    RenderBuffers(deviceContext);

    return true;
}

// Update 함수는 매 프레임의 경과 시간(Delta Time)을 입력받습니다.
// 60fps 기준 약 16ms가 누적되며, 설정된 cycleTime을 넘기면 다음 프레임 이미지를 보여줍니다.
void SpriteClass::Update(float frameTime)
{
    // 매 프레임 경과 시간을 누적
    m_frameTime += frameTime;

    // 누적 시간이 한 프레임의 유지 시간(cycleTime)을 초과했는지 확인
    if (m_frameTime >= m_cycleTime)
    {
        // 초과했다면 시간을 차감하고 다음 텍스처 인덱스로 변경
        m_frameTime -= m_cycleTime;
        m_currentTexture++;

        // 마지막 텍스처라면 다시 처음으로 돌아가 무한 반복(Loop)
        if (m_currentTexture == m_textureCount)
        {
            m_currentTexture = 0;
        }
    }

    return;
}

int SpriteClass::GetIndexCount()
{
    return m_indexCount;
}

// 현재 애니메이션 프레임에 해당하는 텍스처 리소스를 반환합니다.
ID3D11ShaderResourceView* SpriteClass::GetTexture()
{
    return m_Textures[m_currentTexture].GetTexture();
}

bool SpriteClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // 이전 위치 초기화
    m_prevPosX = -1;
    m_prevPosY = -1;

    m_vertexCount = 6;
    m_indexCount = m_vertexCount;

    vertices = new VertexType[m_vertexCount];
    indices = new unsigned long[m_indexCount];

    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // 동적 정점 버퍼 설정 (매 프레임 위치 수정을 허용하기 위함)
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 인덱스 버퍼 설정 (인덱스는 고정이므로 DEFAULT 사용)
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    delete[] vertices;
    vertices = 0;
    delete[] indices;
    indices = 0;

    return true;
}

void SpriteClass::ShutdownBuffers()
{
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

// 위치가 변경되었을 때만 GPU 버퍼를 업데이트하여 성능을 최적화합니다.
bool SpriteClass::UpdateBuffers(ID3D11DeviceContext* deviceContent)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
    {
        return true;
    }

    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;

    vertices = new VertexType[m_vertexCount];

    // 화면 좌표 계산 (가운데가 0,0 기준)
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;
    right = left + (float)m_bitmapWidth;
    top = (float)(m_screenHeight / 2) - (float)m_renderY;
    bottom = top - (float)m_bitmapHeight;

    // 정점 데이터 로드 (두 개의 삼각형으로 사각형 구성)
    vertices[0].position = XMFLOAT3(left, top, 0.0f);  vertices[0].texture = XMFLOAT2(0.0f, 0.0f);
    vertices[1].position = XMFLOAT3(right, bottom, 0.0f); vertices[1].texture = XMFLOAT2(1.0f, 1.0f);
    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  vertices[2].texture = XMFLOAT2(0.0f, 1.0f);
    vertices[3].position = XMFLOAT3(left, top, 0.0f);  vertices[3].texture = XMFLOAT2(0.0f, 0.0f);
    vertices[4].position = XMFLOAT3(right, top, 0.0f);  vertices[4].texture = XMFLOAT2(1.0f, 0.0f);
    vertices[5].position = XMFLOAT3(right, bottom, 0.0f); vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // 버퍼 잠금(Map) 및 데이터 복사
    result = deviceContent->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (VertexType*)mappedResource.pData;
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));
    deviceContent->Unmap(m_vertexBuffer, 0);

    delete[] vertices;
    vertices = 0;

    return true;
}

void SpriteClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride = sizeof(VertexType);
    unsigned int offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// 외부 파일을 열어 애니메이션에 필요한 텍스처 개수, 파일 경로, 속도를 로드합니다.
bool SpriteClass::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    char textureFilename[128];
    std::ifstream fin;
    int i, j;
    char input;
    bool result;

    fin.open(filename);
    if (fin.fail())
    {
        return false;
    }

    // 1. 텍스처 개수 읽기
    fin >> m_textureCount;

    // 개수만큼 텍스처 배열 생성
    m_Textures = new TextureClass[m_textureCount];

    fin.get(input);

    // 2. 각 텍스처 파일 경로 읽기 및 초기화
    for (i = 0; i < m_textureCount; i++)
    {
        j = 0;
        fin.get(input);
        while (input != '\n')
        {
            textureFilename[j] = input;
            j++;
            fin.get(input);
        }
        textureFilename[j] = '\0';

        result = m_Textures[i].Initialize(device, deviceContext, textureFilename);
        if (!result)
        {
            return false;
        }
    }

    // 3. 순환 주기(속도) 읽기 및 초 단위 변환
    fin >> m_cycleTime;
    m_cycleTime = m_cycleTime * 0.001f;

    fin.close();

    // 첫 번째 텍스처 기준으로 스프라이트 크기 설정
    m_bitmapWidth = m_Textures[0].GetWidth();
    m_bitmapHeight = m_Textures[0].GetHeight();

    // 시작 텍스처 설정
    m_currentTexture = 0;

    return true;
}

void SpriteClass::ReleaseTextures()
{
    int i;

    if (m_Textures)
    {
        for (i = 0; i < m_textureCount; i++)
        {
            m_Textures[i].Shutdown();
        }

        delete[] m_Textures;
        m_Textures = 0;
    }

    return;
}

void SpriteClass::SetRenderLocation(int x, int y)
{
    m_renderX = x;
    m_renderY = y;
    return;
}