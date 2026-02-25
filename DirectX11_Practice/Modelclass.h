#pragma once
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

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

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// 아래 함수들은 모델의 정점 및 인덱스 버퍼를 초기화하고 해제하는 역할을 합니다.
	// Render 함수는 모델의 기하학적 구조를 비디오 카드에 올려 컬러 셰이더가 그릴 수 있도록 준비합니다.
	bool Initialize(ID3D11Device*);
	bool Initialize(HWND hwnd, ID3D11Device*, ID3D11DeviceContext*, char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	/*
	* ModelClass에 GetTexture 함수가 추가되었습니다.
	* 이 함수는 모델이 자체적으로 가진 텍스처 리소스(ID3D11ShaderResourceView*)를
	* 이 모델을 그릴 셰이더에 전달하기 위해 사용됩니다.
	*
	* 이전 튜토리얼에서는 모델이 색상만 가지고 있었지만, 이제 텍스처를 사용하기 때문에
	* 각 모델은 셰이더에게 자신이 사용할 텍스처가 무엇인지 알려주어야 합니다.
	*
	* GetTexture 함수는 이러한 역할을 수행하며,
	* 이를 통해 모델과 텍스처 셰이더 간의 중요한 연결고리 역할을 합니다.
	*/
	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	/*
	* ModelClass에는 이제 모델을 렌더링하는 데 사용될 텍스처를
	* 로드하고 해제하기 위한 비공개(private) 함수인
	* LoadTexture와 ReleaseTexture가 추가되었습니다.
	*
	* LoadTexture 함수는 모델의 Initialize 함수에서 호출되어,
	* 특정 파일명의 텍스처를 DirectX 장치를 사용하여 메모리에 로드합니다.
	*
	* ReleaseTexture 함수는 모델의 Shutdown 함수에서 호출되어,
	* 로드된 텍스처 리소스를 해제하고 관련 메모리를 정리합니다.
	*
	* 이 두 함수는 모델 클래스 내에서 텍스처 리소스 관리를
	* 캡슐화하여, 외부 코드가 복잡한 리소스 관리 과정에
	* 관여할 필요가 없게 만듭니다.
	*/
	bool LoadTexture(HWND hwnd, ID3D11Device*, ID3D11DeviceContext*, char*);
	void ReleaseTexture();

	// ModelClass의 private 변수들은 정점 및 인덱스 버퍼이며, 각 버퍼의 크기를 추적하기 위한 두 개의 정수 변수입니다.
	// 참고로, 모든 DirectX 11 버퍼는 일반적으로 ID3D11Buffer라는 일반적인 타입을 사용하며, 처음 생성될 때 버퍼 설명(buffer description)에 의해 더 명확하게 식별됩니다.
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;

	// m_Texture 변수는 이 모델의 텍스처 리소스를 로드, 해제 및 접근하는 데 사용됩니다.
	TextureClass* m_Texture;
};

#endif