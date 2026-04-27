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

	m_Textures = 0;
	m_model = 0;
	m_modelIndices = 0;
	m_indexCount = 0;
	m_vertexCount = 0;

	m_hasEmbeddedTexture = false;
	m_embeddedCompressed = false;
	m_embeddedWidth = 0;
	m_embeddedHeight = 0;
	m_texturePath = "";
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
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
	ReleaseModel();
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
ID3D11ShaderResourceView* ModelClass::GetTexture(int index)
{
	return m_Textures[index].GetTexture();
}

bool ModelClass::InitializeModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename)
{
	bool result;

	std::string fileName = modelFilename;
	std::string ext;

	size_t dotPos = fileName.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		ext = fileName.substr(dotPos);
		for (char& c : ext)
			c = static_cast<char>(tolower(c));
	}

	if (ext == ".txt")
	{
		result = LoadModel_txt(modelFilename);
		if (!result)
			return false;

		// Calculate the tangent and binormal vectors for the model.
		CalculateModelVectors();

		result = InitializeBuffers_txt(device);
		if (!result)
			return false;
	}
	else if (ext == ".fbx")
	{
		result = LoadModel_fbx(modelFilename);
		if (!result)
			return false;

		result = InitializeBuffers_fbx(device);
		if (!result)
			return false;
	}
	else
	{
		return false;
	}
}

bool ModelClass::ProcessTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<char*>& filenames)
{
	bool result;

	int textureCount = static_cast<int>(m_tempTextureList.size());
	if (textureCount > 0)
	{
		m_Textures = new TextureClass[textureCount];

		for (int i = 0; i < textureCount; i++)
		{
			auto& data = m_tempTextureList[i];
			if (data.isEmbedded) {
				if (data.isCompressed) {
					m_Textures[i].InitializeFromMemory(device, deviceContext, data.data.data(), static_cast<int>(data.data.size()));
				}
				else {
					m_Textures[i].InitializeFromRawRGBA(device, deviceContext, data.data.data(), data.width, data.height);
				}
			}
			else {
				// 경로를 이용한 외부 파일 로드
				m_Textures[i].Initialize(device, deviceContext, const_cast<char*>(data.path.c_str()));
			}
		}
	}
	else if (!filenames.empty())
	{
		m_textureCount = static_cast<int>(filenames.size());
		m_Textures = new TextureClass[m_textureCount];

		for (int i = 0; i < m_textureCount; i++)
		{
			result = m_Textures[i].Initialize(device, deviceContext, filenames[i]);
			if (!result) return false;
		}
	}

	return true;
}

// InitializeBuffers 함수는 정점 및 인덱스 버퍼를 생성하는 곳입니다.
// 일반적으로는 모델 파일에서 데이터를 읽어와 버퍼를 생성하지만,
// 이 튜토리얼에서는 단일 삼각형이므로 정점 및 인덱스 데이터를 수동으로 설정합니다.
bool ModelClass::InitializeBuffers_fbx(ID3D11Device* device)
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
	}
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = m_modelIndices[i];
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

bool ModelClass::InitializeBuffers_txt(ID3D11Device* device)
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
		vertices[i].tangent = XMFLOAT3(m_model[i].tx, m_model[i].ty, m_model[i].tz);
		vertices[i].binormal = XMFLOAT3(m_model[i].bx, m_model[i].by, m_model[i].bz);

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

//The ReleaseTexture function will release the texture object that was created and loaded during the LoadTexture function.
void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Textures->Shutdown();
		delete m_Textures;
		m_Textures = 0;
	}

	if (m_Textures)
	{
		m_Textures[0].Shutdown();
		m_Textures[1].Shutdown();

		delete[] m_Textures;
		m_Textures = 0;
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
bool ModelClass::LoadModel_fbx(char* filename)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |           // 모든 폴리곤을 삼각형으로 변환
		aiProcess_ConvertToLeftHanded |   // DX11의 왼손 좌표계로 변환
		aiProcess_GenSmoothNormals |      // 법선이 없으면 생성
		aiProcess_CalcTangentSpace        // 탄젠트 공간 계산 (노멀맵용)
	);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode) {
		// 에러 처리: importer.GetErrorString() 사용
		return false;
	}

	// 1. 첫 번째 메시를 가져오기(단일 모델 기준)
	aiMesh* mesh = pScene->mMeshes[0];
	aiMaterial* material = pScene->mMaterials[mesh->mMaterialIndex];

	// 2. 정점 및 인덱스 개수 설정
	m_vertexCount = mesh->mNumVertices;
	m_indexCount = mesh->mNumFaces * 3; // 삼각형이므로 각 면마다 3개의 인덱스

	// 3. m_model 배열 할당
	m_model = new ModelType[m_vertexCount];
	if (!m_model) return false;

	m_modelIndices = new unsigned long[m_indexCount];
	if (!m_modelIndices) return false;

	// 4. 정점 데이터 읽어오기
	for (unsigned int i = 0; i < m_vertexCount; i++)
	{
		// 위치
		m_model[i].x = mesh->mVertices[i].x;
		m_model[i].y = mesh->mVertices[i].y;
		m_model[i].z = mesh->mVertices[i].z;

		// 텍스쳐 좌표 (UV) - 첫 번째 채널(0) 확인 후 할당
		if (mesh->HasTextureCoords(0))
		{
			m_model[i].tu = mesh->mTextureCoords[0][i].x;
			m_model[i].tv = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			m_model[i].tu = 0.0f;
			m_model[i].tv = 0.0f;
		}

		// 법선 확인
		if (mesh->HasNormals())
		{
			m_model[i].nx = mesh->mNormals[i].x;
			m_model[i].ny = mesh->mNormals[i].y;
			m_model[i].nz = mesh->mNormals[i].z;
		}
		else
		{
			m_model[i].nx = 0.0f;
			m_model[i].ny = 0.0f;
			m_model[i].nz = 0.0f;
		}

		// Tangent과 Bitangent 확인
		if (mesh->HasTangentsAndBitangents())
		{
			m_model[i].tx = mesh->mTangents[i].x;
			m_model[i].ty = mesh->mTangents[i].y;
			m_model[i].tz = mesh->mTangents[i].z;

			m_model[i].bx = mesh->mBitangents[i].x;
			m_model[i].by = mesh->mBitangents[i].y;
			m_model[i].bz = mesh->mBitangents[i].z;
		}
	}

	//실제 face 인덱스 복사
	unsigned int indexOffset = 0;
	for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
		const aiFace& face = mesh->mFaces[f];

		if (face.mNumIndices != 3) continue;

		m_modelIndices[indexOffset++] = face.mIndices[0];
		m_modelIndices[indexOffset++] = face.mIndices[1];
		m_modelIndices[indexOffset++] = face.mIndices[2];
	}

	m_indexCount = indexOffset; // 실제로 복사된 인덱스 수로 업데이트

	// --- 텍스처 처리 로직 시작 ---

	// 로드하고자 하는 텍스처 타입들을 정의합니다.
	aiTextureType types[] = {
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS
	};
	// 임시로 데이터를 담을 vector (Initialize에서 사용할 용도)
	m_tempTextureList.clear();

	for (auto type : types)
	{
		if (material->GetTextureCount(type) > 0)
		{
			aiString texPath;
			if (material->GetTexture(type, 0, &texPath) == AI_SUCCESS)
			{
				TempTextureData tempData;
				const aiTexture* embeddedTex = pScene->GetEmbeddedTexture(texPath.C_Str());

				if (embeddedTex)
				{
					tempData.isEmbedded = true;
					if (embeddedTex->mHeight == 0)
					{
						tempData.isCompressed = true;
						tempData.width = embeddedTex->mWidth; // 전체 바이트 크기
						tempData.height = 0; // 구분용
						const unsigned char* src = reinterpret_cast<const unsigned char*>(embeddedTex->pcData);
						tempData.data.assign(src, src + embeddedTex->mWidth);
					}
					else
					{
						tempData.isCompressed = false;
						tempData.width = embeddedTex->mWidth;
						tempData.height = embeddedTex->mHeight;
						size_t texelCount = static_cast<size_t>(embeddedTex->mWidth) * static_cast<size_t>(embeddedTex->mHeight);
						size_t byteSize = texelCount * 4; // RGBA8888
						const unsigned char* src = reinterpret_cast<const unsigned char*>(embeddedTex->pcData);
						tempData.data.assign(src, src + byteSize);
					}
				}
				else
				{
					tempData.isEmbedded = false;
					tempData.path = texPath.C_Str();
				}
				m_tempTextureList.push_back(tempData);
			}
		}
	}

	return true;
}

bool ModelClass::LoadModel_txt(char* filename)
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
	m_modelIndices = new unsigned long[m_indexCount];
	if (!m_modelIndices) return false;

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

void ModelClass::CalculateModelVectors()
{
	int faceCount, i, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;


	// Calculate the number of faces in the model.
	faceCount = m_vertexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent and binormal vectors.
	for (i = 0; i < faceCount; i++)
	{
		// Get the three vertices for this face from the model.
		vertex1.x = m_model[index].x;
		vertex1.y = m_model[index].y;
		vertex1.z = m_model[index].z;
		vertex1.tu = m_model[index].tu;
		vertex1.tv = m_model[index].tv;
		index++;

		vertex2.x = m_model[index].x;
		vertex2.y = m_model[index].y;
		vertex2.z = m_model[index].z;
		vertex2.tu = m_model[index].tu;
		vertex2.tv = m_model[index].tv;
		index++;

		vertex3.x = m_model[index].x;
		vertex3.y = m_model[index].y;
		vertex3.z = m_model[index].z;
		vertex3.tu = m_model[index].tu;
		vertex3.tv = m_model[index].tv;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the tangent and binormal for this face back in the model structure.
		m_model[index - 1].tx = tangent.x;
		m_model[index - 1].ty = tangent.y;
		m_model[index - 1].tz = tangent.z;
		m_model[index - 1].bx = binormal.x;
		m_model[index - 1].by = binormal.y;
		m_model[index - 1].bz = binormal.z;

		m_model[index - 2].tx = tangent.x;
		m_model[index - 2].ty = tangent.y;
		m_model[index - 2].tz = tangent.z;
		m_model[index - 2].bx = binormal.x;
		m_model[index - 2].by = binormal.y;
		m_model[index - 2].bz = binormal.z;

		m_model[index - 3].tx = tangent.x;
		m_model[index - 3].ty = tangent.y;
		m_model[index - 3].tz = tangent.z;
		m_model[index - 3].bx = binormal.x;
		m_model[index - 3].by = binormal.y;
		m_model[index - 3].bz = binormal.z;
	}

	return;
}

void ModelClass::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}