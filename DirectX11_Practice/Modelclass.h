#pragma once
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "TextureClass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
	// VertexType 구조체에 이제 조명을 고려한
	// 법선 벡터가 추가되었습니다.
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	// 다음 변경 사항은 모델 파일 형식을 표현하기 위한 새로운 구조체의 추가입니다.
	// 이 구조체의 이름은 ModelType입니다.
	// 이 구조체는 파일 형식과 동일하게 위치(position), 텍스처(texture), 법선(normal) 벡터를 포함합니다.
	struct ModelType
	{
		float x, y, z;   // 위치 좌표
		float tu, tv;    // 텍스처 좌표
		float nx, ny, nz;// 법선 벡터
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// 아래 함수들은 모델의 정점 및 인덱스 버퍼를 초기화하고 해제하는 역할을 합니다.
	// Render 함수는 모델의 기하학적 구조를 비디오 카드에 올려 컬러 셰이더가 그릴 수 있도록 준비합니다.
	
	// Initialize 함수는 이제 로드할 모델 파일 이름과 텍스처 파일 이름을 입력으로 받습니다.
	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*, char*);
	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*, char*, char*);
	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*, char*, char*, char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture(int);
private:
	bool InitializeBuffers(ID3D11Device*);
	bool InitializeBuffers_2(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, char*);
	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, char*, char*);
	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, char*, char*, char*);
	void ReleaseTexture();

	// 텍스트 파일에서 모델 데이터를 로드하고 해제하기 위한 새로운 함수들입니다.
	bool LoadModel(char*);
	bool LoadModel_2(char* filename);
	void ReleaseModel();
private:
	// ModelClass의 private 변수들은 정점 및 인덱스 버퍼이며, 각 버퍼의 크기를 추적하기 위한 두 개의 정수 변수입니다.
	// 참고로, 모든 DirectX 11 버퍼는 일반적으로 ID3D11Buffer라는 일반적인 타입을 사용하며, 처음 생성될 때 버퍼 설명(buffer description)에 의해 더 명확하게 식별됩니다.
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;

	// m_Texture 변수는 이 모델의 텍스처 리소스를 로드, 해제 및 접근하는 데 사용됩니다.
	TextureClass* m_Texture;
	TextureClass* m_Textures;
	// 마지막 변경 사항은 m_model이라는 새로운 private 변수입니다.
	// 이 변수는 ModelType 구조체 배열로 선언됩니다.
	// 이 변수는 모델 데이터를 텍스트 파일에서 읽어온 후,
	// 버텍스 버퍼에 저장하기 전에 임시로 보관하는 용도로 사용됩니다.
	ModelType* m_model;
	unsigned long* m_modelIndices;

private:
	std::string m_texturePath;
	bool m_hasEmbeddedTexture;
	bool m_embeddedCompressed;   // mHeight == 0 이면 true
	std::vector<unsigned char> m_embeddedTextureData;
	int m_embeddedWidth;
	int m_embeddedHeight;
};

#endif