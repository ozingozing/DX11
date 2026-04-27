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
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

	// 다음 변경 사항은 모델 파일 형식을 표현하기 위한 새로운 구조체의 추가입니다.
	// 이 구조체의 이름은 ModelType입니다.
	// 이 구조체는 파일 형식과 동일하게 위치(position), 텍스처(texture), 법선(normal) 벡터를 포함합니다.
	struct ModelType
	{
		float x, y, z;   // 위치 좌표
		float tu, tv;    // 텍스처 좌표
		float nx, ny, nz;// 법선 벡터
		float tx, ty, tz;// 탄젠트 벡터
		float bx, by, bz;// 바이노멀 벡터
	};

	//다음 두 구조는 탄젠트와 바이노멀을 계산하는 데 사용됩니다.
	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct TempTextureData {
		bool isEmbedded;
		bool isCompressed;
		int width, height;
		std::string path;
		std::vector<unsigned char> data; // 내장 텍스처용 데이터
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// 아래 함수들은 모델의 정점 및 인덱스 버퍼를 초기화하고 해제하는 역할을 합니다.
	// Render 함수는 모델의 기하학적 구조를 비디오 카드에 올려 컬러 셰이더가 그릴 수 있도록 준비합니다.

	// 가변 인자를 받는 통합 Initialize 함수
	template<typename... Args>
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, Args... textureFilenames)
	{
		// 1. 모델 파일(FBX 등) 로드 공통 로직 실행
		if (!InitializeModel(device, deviceContext, modelFilename))
		{
			return false;
		}

		// 2. 가변 인자로 들어온 텍스처 파일들을 vector로 묶어서 처리
		std::vector<char*> textures = { textureFilenames... };

		// 텍스처가 하나도 전달되지 않았더라도 내부 로직(내장 텍스처 등)을 위해 호출
		return ProcessTextures(device, deviceContext, textures);
	}
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture(int);
private:
	// 내부 모델 로드 로직 (기존 Initialize의 앞부분)
	bool InitializeModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename);

	// 내부 텍스처 처리 로직 (가변 인자로 받은 파일들 처리)
	bool ProcessTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<char*>& filenames);

	bool InitializeBuffers_fbx(ID3D11Device*);
	bool InitializeBuffers_txt(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	void ReleaseTexture();

	// 텍스트 파일에서 모델 데이터를 로드하고 해제하기 위한 새로운 함수들입니다.
	bool LoadModel_fbx(char*);
	bool LoadModel_txt(char* filename);
	void ReleaseModel();

	//모델의 접선 tangent와 binormal벡터를 계산하는 두 가지 새로운 함수를 추가했습니다.
	void CalculateModelVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);
private:
	// ModelClass의 private 변수들은 정점 및 인덱스 버퍼이며, 각 버퍼의 크기를 추적하기 위한 두 개의 정수 변수입니다.
	// 참고로, 모든 DirectX 11 버퍼는 일반적으로 ID3D11Buffer라는 일반적인 타입을 사용하며, 처음 생성될 때 버퍼 설명(buffer description)에 의해 더 명확하게 식별됩니다.
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;

	// m_Texture 변수는 이 모델의 텍스처 리소스를 로드, 해제 및 접근하는 데 사용됩니다.
	TextureClass* m_Texture;
	TextureClass* m_Textures;
	int m_textureCount;	// 텍스처 개수를 추적하기 위한 변수 추가 권장
	// 마지막 변경 사항은 m_model이라는 새로운 private 변수입니다.
	// 이 변수는 ModelType 구조체 배열로 선언됩니다.
	// 이 변수는 모델 데이터를 텍스트 파일에서 읽어온 후,
	// 버텍스 버퍼에 저장하기 전에 임시로 보관하는 용도로 사용됩니다.
	ModelType* m_model;
	unsigned long* m_modelIndices;

private:
	vector<TempTextureData> m_tempTextureList;
	std::string m_texturePath;
	bool m_hasEmbeddedTexture;
	bool m_embeddedCompressed;   // mHeight == 0 이면 true
	std::vector<unsigned char> m_embeddedTextureData;
	int m_embeddedWidth;
	int m_embeddedHeight;
};

#endif