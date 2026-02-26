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
	m_model = 0;
	m_indexCount = 0;
	m_vertexCount = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}

// 이제 Initialize 함수는 로드할 모델 파일 이름을
// 입력으로 받습니다. 그리고 Initialize 함수 내에서
// 먼저 새로운 LoadModel 함수를 호출합니다.
// 이 함수는 제공된 파일 이름에서 모델 데이터를
// 로드하여 새로운 m_model 배열에 저장합니다.
// 이 모델 배열이 채워지면, 이를 기반으로
// 정점 버퍼와 인덱스 버퍼를 생성할 수 있습니다.
// InitializeBuffers 함수는 이제 이 모델 데이터에 의존하므로,
// 함수들을 올바른 순서로 호출해야 합니다.
bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, char* textureFilename)
{
	bool result;


	// Load in the model data.
	result = LoadModel(modelFilename);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Load the texture for this model.
	result = LoadTexture(device, deviceContext, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

// Shutdown 함수는 정점 및 인덱스 버퍼 종료 함수를 호출합니다.
void ModelClass::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();
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
	int i;

	// Create the vertex array
	vertices = new VertexType[m_vertexCount];
	// Create the index array
	indices = new unsigned long[m_indexCount];
	// Load rhe vertex and index array with data.
	for (i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}

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
bool ModelClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;


	// Create and initialize the texture object.
	m_Texture = new TextureClass;

	result = m_Texture->Initialize(device, deviceContext, filename);
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

// 이 함수는 텍스트 파일에서 모델 데이터를
// m_model 배열 변수로 불러오는 기능을 담당하는
// 새로운 LoadModel 함수입니다.
// 먼저 텍스트 파일을 열고 정점 개수를 읽어옵니다.
// 정점 개수를 읽어온 후에는 ModelType 배열을 생성하고
// 각 행을 배열에 읽어들입니다.
// 이제 이 함수에서 정점 개수와 인덱스 개수가 모두 설정됩니다.
bool ModelClass::LoadModel(char* filename)
{
	ifstream fin;
	char input;
	int i;

	// Open the model file.
	fin.open(filename);

	// if it could not open the file then exit.
	if (fin.fail())
	{
		return false;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = new ModelType[m_vertexCount];

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i < m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// Close the model file.
	fin.close();

	return true;
}

// The ReleaseModel function handles
// deleting the model data array.
void ModelClass::ReleaseModel()
{
	// Release the model data.
	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}
	return;
}