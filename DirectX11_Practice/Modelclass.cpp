#include "Modelclass.h"

// 클래스 생성자는 정점 버퍼와 인덱스 버퍼 포인터를 null로 초기화합니다.
ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}

// Initialize 함수는 정점 버퍼와 인덱스 버퍼를 위한 초기화 함수들을 호출합니다.
bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;


	// 정점 버퍼와 인덱스 버퍼를 초기화합니다.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}

// Shutdown 함수는 정점 버퍼와 인덱스 버퍼를 위한 해제 함수들을 호출합니다.
void ModelClass::Shutdown()
{
	// 정점 버퍼와 인덱스 버퍼를 해제합니다.
	ShutdownBuffers();

	return;
}

// Render 함수는 ApplicationClass::Render 함수에서 호출됩니다.
// 이 함수는 RenderBuffers를 호출하여 정점 버퍼와 인덱스 버퍼를 그래픽 파이프라인에 배치하며,
// 이를 통해 컬러 셰이더가 해당 버퍼들을 렌더링할 수 있게 합니다.
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 그리기 준비를 위해 정점 버퍼와 인덱스 버퍼를 그래픽 파이프라인에 배치합니다.
	RenderBuffers(deviceContext);

	return;
}

// GetIndexCount 함수는 모델의 인덱스 개수를 반환합니다.
// 컬러 셰이더가 이 모델을 그리기 위해 이 정보가 필요합니다.
int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

// InitializeBuffers 함수는 정점 버퍼와 인덱스 버퍼의 생성을 처리합니다.
// 보통은 모델 파일을 읽어와서 버퍼를 생성하지만, 이 튜토리얼에서는 단일 삼각형만 다루므로 
// 정점 및 인덱스 버퍼의 좌표들을 수동으로 설정하겠습니다.
bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// 먼저 최종 버퍼를 채우는 데 사용할 정점 및 인덱스 데이터를 담을 임시 배열 두 개를 생성합니다.

	// 정점 배열에 사용할 정점 개수를 설정합니다.
	m_vertexCount = 3;

	// 인덱스 배열에 사용할 인덱스 개수를 설정합니다.
	m_indexCount = 3;

	// 정점 배열을 생성합니다.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// 인덱스 배열을 생성합니다.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// 이제 정점 및 인덱스 배열에 삼각형의 세 지점과 각 지점에 대한 인덱스를 채웁니다.
	// 점을 생성할 때 그리는 순서에 맞춰 '시계 방향'으로 생성해야 함에 유의하세요.
	// 만약 반시계 방향으로 생성하면, GPU는 삼각형이 반대쪽을 향하고 있다고 판단하여 
	// 후면 컬링(Back-face Culling)에 의해 그리지 않게 됩니다.
	// GPU로 정점을 보내는 순서는 매우 중요하므로 항상 기억해두어야 합니다.
	// 색상 또한 정점 구조체의 일부이므로 여기서 함께 설정하며, 여기서는 초록색으로 설정했습니다.

	// 정점 배열에 데이터를 로드합니다.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // 왼쪽 아래
	vertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);   // 위쪽 중앙
	vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // 오른쪽 아래
	vertices[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	// 인덱스 배열에 데이터를 로드합니다.
	indices[0] = 0;  // 왼쪽 아래
	indices[1] = 1;  // 위쪽 중앙
	indices[2] = 2;  // 오른쪽 아래

	// 정점 배열과 인덱스 배열이 채워졌으므로, 이를 사용하여 정점 버퍼와 인덱스 버퍼를 생성할 수 있습니다.
	// 두 버퍼 모두 동일한 방식으로 생성됩니다. 먼저 버퍼의 설명(Description)을 작성합니다.
	// 설명에서 ByteWidth(버퍼의 크기)와 BindFlags(버퍼의 타입)가 정확하게 채워졌는지 확인해야 합니다.
	// 설명 작성이 끝나면 이전에 생성한 정점 또는 인덱스 배열을 가리키는 서브리소스 포인터를 설정합니다.
	// 이 설명과 서브리소스 포인터를 사용하여 D3D 디바이스의 CreateBuffer를 호출하면 새 버퍼에 대한 포인터가 반환됩니다.

	// 정적 정점 버퍼의 설명을 설정합니다.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 서브리소스 구조체에 정점 데이터에 대한 포인터를 제공합니다.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 생성합니다.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 정적 인덱스 버퍼의 설명을 설정합니다.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 서브리소스 구조체에 인덱스 데이터에 대한 포인터를 제공합니다.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 정점 버퍼와 인덱스 버퍼가 성공적으로 생성되었다면, 데이터가 버퍼로 복사되었으므로 
	// 더 이상 필요 없는 정점 및 인덱스 배열을 삭제합니다.

	// 정점 및 인덱스 버퍼가 생성되고 로드되었으므로 배열들을 해제합니다.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

// ShutdownBuffers 함수는 InitializeBuffers 함수에서 생성되었던
// 정점 버퍼(Vertex Buffer)와 인덱스 버퍼(Index Buffer)를 해제합니다.
void ModelClass::ShutdownBuffers()
{
	// 인덱스 버퍼를 해제합니다.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// 정점 버퍼를 해제합니다.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

// RenderBuffers 함수는 Render 함수로부터 호출됩니다. 
// 이 함수의 목적은 GPU의 입력 어셈블러(Input Assembler)에 정점 버퍼와 인덱스 버퍼를 활성 상태로 설정하는 것입니다.
// GPU에 활성화된 정점 버퍼가 있으면, 셰이더를 사용하여 해당 버퍼를 렌더링할 수 있습니다.
// 또한 이 함수는 버퍼를 삼각형, 선, 팬(fan) 등 어떤 형태로 그릴지 정의합니다.
// 이 튜토리얼에서는 입력 어셈블러에 정점 및 인덱스 버퍼를 활성화하고, 
// IASetPrimitiveTopology 함수를 사용하여 GPU에 이 버퍼들을 '삼각형 리스트' 형태로 그리도록 지시합니다.
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// 정점 버퍼의 보폭(stride)과 오프셋(offset)을 설정합니다.
	stride = sizeof(VertexType);
	offset = 0;

	// 렌더링될 수 있도록 입력 어셈블러에 정점 버퍼를 활성 상태로 설정합니다.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 렌더링될 수 있도록 입력 어셈블러에 인덱스 버퍼를 활성 상태로 설정합니다.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 이 정점 버퍼로부터 렌더링할 기본도형(Primitive) 타입을 설정합니다. 여기서는 삼각형 리스트입니다.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}