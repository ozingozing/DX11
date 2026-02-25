////////////////////////////////////////////////////////////////////////////////
// filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#define CIRCLE_SEGMENTS 30
// 클래스 생성자는 정점 및 인덱스 버퍼 포인터를 null로 초기화합니다.
ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;

	m_Texture = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}

// Initialize 함수는 정점 및 인덱스 버퍼 초기화 함수를 호출합니다.
bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;

	// 정점 및 인덱스 버퍼를 초기화합니다.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}

bool ModelClass::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename)
{
	bool result;

	// 버텍스 및 인덱스 버퍼를 초기화합니다.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	// Initialize 함수는 이제 텍스처를 로드하는 새로운 비공개(private) 함수를 호출합니다.
	// 이 모델에 사용할 텍스처를 로드합니다.
	result = LoadTexture(hwnd, device, deviceContext, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

// Shutdown 함수는 정점 및 인덱스 버퍼 종료 함수를 호출합니다.
void ModelClass::Shutdown()
{
	/*
	* Shutdown 함수는 이제 초기화 중에 로드되었던 텍스처 객체를
	* 해제하기 위해 새로운 비공개(private) 함수를 호출합니다.
	*/
	// Release the model texture.
	ReleaseTexture();
	// 정점 및 인덱스 버퍼를 종료합니다.
	ShutdownBuffers();

	return;
}

// Render 함수는 ApplicationClass::Render 함수에서 호출됩니다.
// 이 함수는 RenderBuffers를 호출하여 정점 및 인덱스 버퍼를 그래픽 파이프라인에 배치하고,
// 컬러 셰이더가 이들을 렌더링할 수 있도록 준비합니다.
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 정점 및 인덱스 버퍼를 그래픽 파이프라인에 배치하여 그리기를 준비합니다.
	RenderBuffers(deviceContext);

	return;
}

// GetIndexCount는 모델의 인덱스 수를 반환합니다.
// 컬러 셰이더는 이 모델을 그리기 위해 이 정보가 필요합니다.
int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

/*
* GetTexture는 모델의 텍스처 리소스를 반환하는 새로운 함수입니다.
* 텍스처 셰이더는 이 모델을 렌더링하기 위해 이 텍스처에 접근해야 합니다.
*/
ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

// InitializeBuffers 함수는 정점 및 인덱스 버퍼를 생성하는 곳입니다.
// 일반적으로는 모델 파일에서 데이터를 읽어와 버퍼를 생성하지만,
// 이 튜토리얼에서는 단일 삼각형이므로 정점 및 인덱스 데이터를 수동으로 설정합니다.
bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// 최종 버퍼에 데이터를 채우기 위해 사용할 정점 및 인덱스 데이터 임시 배열을 먼저 생성합니다.
	// 정점 배열의 정점 수를 설정합니다.
	m_vertexCount = 4;

	// 인덱스 배열의 인덱스 수를 설정합니다.
	m_indexCount = 6;

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

	// InitializeBuffers 함수의 유일한 변경 사항은
	// 정점 설정 부분입니다. 이제 각 정점에는
	// 조명 계산을 위한 법선이 연결됩니다.
	// 법선은 다각형의 면에 수직인 선으로,
	// 면이 가리키는 정확한 방향을 계산할 수 있도록 합니다.
	// 간단하게 설명하기 위해 각 정점의 Z축 성분을 -1.0f로
	// 설정하여 법선이 보는 방향을 향하도록 했습니다.
	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);
	vertices[0].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].texture = XMFLOAT2(0.5f, 0.0f);
	vertices[1].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);
	vertices[2].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.

	// 정점 및 인덱스 배열이 채워졌으므로 이제 이를 사용하여 정점 버퍼와 인덱스 버퍼를 생성할 수 있습니다.
	// 두 버퍼를 생성하는 방식은 동일합니다. 먼저 버퍼에 대한 설명(description)을 채웁니다.
	// 설명에서 ByteWidth(버퍼의 크기)와 BindFlags(버퍼의 타입)가 올바르게 채워졌는지 확인해야 합니다.
	// 설명이 채워진 후에는 이전에 생성한 정점 또는 인덱스 배열을 가리킬 서브리소스 포인터도 채워야 합니다.
	// 설명과 서브리소스 포인터를 사용하여 D3D 장치를 이용해 CreateBuffer를 호출하면 새 버퍼에 대한 포인터가 반환됩니다.
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

	// 정점 버퍼와 인덱스 버퍼가 생성되고 로드되었으므로, 더 이상 필요하지 않은 정점 및 인덱스 배열을 해제합니다.
	// 데이터가 버퍼에 복사되었기 때문입니다.
	// 이제 정점 및 인덱스 배열을 해제합니다.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

// ShutdownBuffers 함수는 InitializeBuffers 함수에서 생성된 정점 및 인덱스 버퍼를 단순히 해제합니다.
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

//bool ModelClass::InitializeBuffers(ID3D11Device* device)
//{
//	VertexType* vertices;
//	unsigned long* indices;
//	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
//	D3D11_SUBRESOURCE_DATA vertexData, indexData;
//	HRESULT result;
//
//	// 원의 중심점과 주변 정점들을 포함하여 정점 수를 설정합니다.
//	m_vertexCount = CIRCLE_SEGMENTS + 1; // 중심점 + 원 둘레의 정점 수
//
//	// 삼각형 개수 * 3개의 인덱스 수를 설정합니다.
//	m_indexCount = CIRCLE_SEGMENTS * 3;
//
//	// 정점 배열을 생성합니다.
//	vertices = new VertexType[m_vertexCount];
//	if (!vertices)
//	{
//		return false;
//	}
//
//	// 인덱스 배열을 생성합니다.
//	indices = new unsigned long[m_indexCount];
//	if (!indices)
//	{
//		return false;
//	}
//
//	// === 1. 정점 데이터 계산 ===
//	// 원의 중심 정점을 설정합니다.
//	vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f); // 원점
//	vertices[0].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 흰색
//
//	// 원의 둘레를 따라 정점들을 계산합니다.
//	float radius = 1.0f;
//	float angleIncrement = XM_2PI / (float)CIRCLE_SEGMENTS; // 2PI (360도)를 세그먼트 수로 나눔
//
//	for (int i = 0; i < CIRCLE_SEGMENTS; i++)
//	{
//		float currentAngle = i * angleIncrement;
//		float x = radius * cos(currentAngle);
//		float y = radius * sin(currentAngle);
//
//		vertices[i + 1].position = XMFLOAT3(x, -y, 0.0f);
//		vertices[i + 1].color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 빨간색 원
//	}
//
//	// === 2. 인덱스 데이터 계산 ===
//	// 부채꼴 모양의 삼각형을 구성하기 위한 인덱스를 설정합니다.
//	for (int i = 0; i < CIRCLE_SEGMENTS; i++)
//	{
//		indices[i * 3 + 0] = 0;             // 모든 삼각형의 첫 번째 인덱스는 중심점
//		indices[i * 3 + 1] = i + 1;         // 원 둘레의 현재 정점
//		indices[i * 3 + 2] = (i + 1) % CIRCLE_SEGMENTS + 1; // 원 둘레의 다음 정점 (마지막 정점은 첫 번째 정점과 연결)
//	}
//
//	// === 3. 버퍼 생성 (이하 코드는 사각형과 동일) ===
//	// 정적 정점 버퍼의 설명을 설정합니다.
//	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
//	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vertexBufferDesc.CPUAccessFlags = 0;
//	vertexBufferDesc.MiscFlags = 0;
//	vertexBufferDesc.StructureByteStride = 0;
//
//	// 서브리소스 구조체에 정점 데이터에 대한 포인터를 제공합니다.
//	vertexData.pSysMem = vertices;
//	vertexData.SysMemPitch = 0;
//	vertexData.SysMemSlicePitch = 0;
//
//	// 정점 버퍼를 생성합니다.
//	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
//	if (FAILED(result))
//	{
//		return false;
//	}
//
//	// 정적 인덱스 버퍼의 설명을 설정합니다.
//	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
//	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	indexBufferDesc.CPUAccessFlags = 0;
//	indexBufferDesc.MiscFlags = 0;
//	indexBufferDesc.StructureByteStride = 0;
//
//	// 서브리소스 구조체에 인덱스 데이터에 대한 포인터를 제공합니다.
//	indexData.pSysMem = indices;
//	indexData.SysMemPitch = 0;
//	indexData.SysMemSlicePitch = 0;
//
//	// 인덱스 버퍼를 생성합니다.
//	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
//	if (FAILED(result))
//	{
//		return false;
//	}
//
//	// 배열 해제
//	delete[] vertices;
//	vertices = 0;
//	delete[] indices;
//	indices = 0;
//
//	return true;
//}

// RenderBuffers는 Render 함수에서 호출됩니다.
// 이 함수의 목적은 정점 및 인덱스 버퍼를 GPU의 입력 어셈블러에 활성화하는 것입니다.
// GPU에 활성 정점 버퍼가 있으면 셰이더를 사용하여 해당 버퍼를 렌더링할 수 있습니다.
// 이 함수는 또한 버퍼를 어떻게 그릴지(예: 삼각형, 선, 팬 등) 정의합니다.
// 이 튜토리얼에서는 정점 및 인덱스 버퍼를 입력 어셈블러에 활성화하고,
// IASetPrimitiveTopology DirectX 함수를 사용하여 버퍼가 삼각형으로 그려져야 함을 GPU에 알려줍니다.
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// 정점 버퍼의 스트라이드와 오프셋을 설정합니다.
	stride = sizeof(VertexType);
	offset = 0;

	// 렌더링할 수 있도록 정점 버퍼를 입력 어셈블러에 활성화합니다.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 렌더링할 수 있도록 인덱스 버퍼를 입력 어셈블러에 활성화합니다.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 이 정점 버퍼에서 렌더링되어야 할 기본 도형의 타입을 설정합니다. 이 경우 삼각형입니다.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

//LoadTexture is a new private function that will create the texture object and then initialize it with the input file name provided.This function is called during initialization.
bool ModelClass::LoadTexture(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;


	// Create and initialize the texture object.
	m_Texture = new TextureClass;

	result = m_Texture->Initialize(hwnd, device, deviceContext, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

//The ReleaseTexture function will release the texture object that was created and loaded during the LoadTexture function.
void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}