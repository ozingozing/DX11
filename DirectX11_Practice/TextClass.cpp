///////////////////////////////////////////////////////////////////////////////
// 파일명: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"

TextClass::TextClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
}

TextClass::TextClass(const TextClass& other)
{
}

TextClass::~TextClass()
{
}

// Initialize 함수는 문장을 출력하는 데 필요한 정보를 저장하고,
// InitializeBuffers를 호출하여 실제 문장 형태를 구축합니다.
bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, int maxLength, FontClass* Font, char* text,
    int positionX, int positionY, float red, float green, float blue)
{
    bool result;

    // 화면의 가로, 세로 해상도를 저장합니다.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // 문장의 최대 길이를 저장합니다.
    m_maxLength = maxLength;

    // 문장을 초기화합니다. (버퍼 생성 및 데이터 입력)
    result = InitializeBuffers(device, deviceContext, Font, text, positionX, positionY, red, green, blue);
    if (!result)
    {
        return false;
    }

    return true;
}

// Shutdown 함수는 문장 렌더링에 사용된 버퍼들을 해제합니다.
void TextClass::Shutdown()
{
    // 정점 및 인덱스 버퍼를 해제합니다.
    ShutdownBuffers();

    return;
}

// Render 함수는 텍스트 문장을 그립니다.
void TextClass::Render(ID3D11DeviceContext* deviceContext)
{
    // 그리기 준비를 위해 정점 및 인덱스 버퍼를 그래픽 파이프라인에 배치합니다.
    RenderBuffers(deviceContext);

    return;
}

// GetIndexCount 함수는 텍스트 문장의 인덱스 개수를 반환합니다.
int TextClass::GetIndexCount()
{
    return m_indexCount;
}

// InitializeBuffers 함수는 텍스트 렌더링을 위한 정점 버퍼를 실제로 생성하는 곳입니다.
bool TextClass::InitializeBuffers(ID3D11Device* device, ID3D11DeviceContext* deviceContext, FontClass* Font, char* text, int positionX, int positionY, float red, float green, float blue)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // 1. 먼저 정점 및 인덱스 배열을 초기화하고 생성합니다.

    // 정점 및 인덱스 개수를 설정합니다. (글자당 6개의 정점 사용)
    m_vertexCount = 6 * m_maxLength;
    m_indexCount = m_vertexCount;

    // 정점 배열을 생성합니다.
    vertices = new VertexType[m_vertexCount];

    // 인덱스 배열을 생성합니다.
    indices = new unsigned long[m_indexCount];

    // 정점 배열을 0으로 초기화합니다.
    std::memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // 인덱스 배열을 초기화합니다. (0, 1, 2, 3...)
    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // 2. 생성된 배열을 사용하여 DirectX 정점 및 인덱스 버퍼를 생성합니다.
    // 아직은 실제 렌더링 데이터가 들어있지 않은 빈 상태입니다.

    // 동적(Dynamic) 정점 버퍼에 대한 설명을 설정합니다.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 정점 데이터 포인터를 전달합니다.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 정점 버퍼를 생성합니다.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 정적(Default) 인덱스 버퍼에 대한 설명을 설정합니다.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // 서브리소스 구조체에 인덱스 데이터 포인터를 전달합니다.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼를 생성합니다.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 더 이상 필요 없는 시스템 메모리 상의 정점 배열을 삭제합니다.
    delete[] vertices;
    vertices = 0;

    // 더 이상 필요 없는 시스템 메모리 상의 인덱스 배열을 삭제합니다.
    delete[] indices;
    indices = 0;

    // 3. DirectX 버퍼가 생성되었으므로, UpdateText 함수를 호출하여 
    // 실제 문장의 정점/인덱스 데이터를 버퍼에 채웁니다.
    result = UpdateText(deviceContext, Font, text, positionX, positionY, red, green, blue);
    if (!result)
    {
        return false;
    }

    return true;
}

// ShutdownBuffers 함수는 정점 및 인덱스 버퍼 리소스를 해제합니다.
void TextClass::ShutdownBuffers()
{
    // 인덱스 버퍼 해제
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // 정점 버퍼 해제
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

// UpdateText 함수는 문장 렌더링 데이터를 구축하고 이를 버퍼에 저장합니다.
bool TextClass::UpdateText(ID3D11DeviceContext* deviceContext, FontClass* Font, char* text, int positionX, int positionY, float red, float green, float blue)
{
    int numLetters;
    VertexType* vertices;
    float drawX, drawY;
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;

    // 1. 먼저 이 문장이 렌더링될 색상을 저장합니다.
    m_pixelColor = XMFLOAT4(red, green, blue, 1.0f);

    // 2. 입력된 문장이 처음에 할당한 버퍼 크기에 맞는지 확인합니다.
    numLetters = (int)strlen(text);

    // 버퍼 오버플로우 방지 체크
    if (numLetters > m_maxLength)
    {
        return false;
    }

    // 3. 렌더링 데이터를 임시로 저장할 빈 정점 배열을 생성합니다.
    vertices = new VertexType[m_vertexCount];

    // 정점 배열을 0으로 초기화합니다.
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // 4. 문장이 그려지기 시작할 화면 상의 초기 좌표(X, Y)를 계산합니다.
    drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
    drawY = (float)((m_screenHeight / 2) - positionY);

    // 5. FontClass 객체를 사용하여 정점 렌더링 데이터를 구축합니다.
    // 텍스트 내용과 위치 정보를 보내면 FontClass가 vertices 배열을 채워줍니다.
    Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

    // 6. 완성된 정점 데이터를 DirectX 버퍼로 복사하여 렌더링 준비를 마칩니다.

    // 쓰기 작업을 위해 정점 버퍼를 잠급니다(Lock).
    result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 정점 버퍼의 데이터 시작 지점 주소를 얻습니다.
    verticesPtr = (VertexType*)mappedResource.pData;

    // 데이터를 정점 버퍼로 복사합니다.
    memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // 정점 버퍼 잠금을 해제합니다(Unlock).
    deviceContext->Unmap(m_vertexBuffer, 0);

    // 시스템 메모리의 임시 정점 배열을 삭제합니다.
    delete[] vertices;
    vertices = 0;

    return true;
}

// RenderBuffers 함수는 문장 데이터를 담고 있는 정점 및 인덱스 버퍼를 렌더링합니다.
void TextClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride, offset;

    // 정점 버퍼의 간격(stride)과 오프셋을 설정합니다.
    stride = sizeof(VertexType);
    offset = 0;

    // 입력 조립기(Input Assembler)에 정점 버퍼를 활성 상태로 설정합니다.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // 입력 조립기에 인덱스 버퍼를 활성 상태로 설정합니다.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 이 정점 버퍼로부터 그려질 기본 도형 타입을 설정합니다 (여기서는 삼각형 리스트).
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// GetPixelColor 함수는 문장이 그려져야 할 색상 값을 반환합니다.
XMFLOAT4 TextClass::GetPixelColor()
{
    return m_pixelColor;
}