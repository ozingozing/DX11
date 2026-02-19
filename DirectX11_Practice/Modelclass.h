#pragma once
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	// 이 ModelClass의 정점 버퍼에서 사용할 정점 타입의 정의입니다.
	// 이 구조체 정의는 나중에 살펴볼 ColorShaderClass의 레이아웃 구성과 
	// 반드시 일치해야 하므로 주의가 필요합니다.
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// 아래 함수들은 모델의 정점 버퍼 및 인덱스 버퍼의 초기화와 해제를 담당합니다.
	// Render 함수는 컬러 셰이더가 그릴 수 있도록 모델 기하 구조를 그래픽 카드에 배치합니다.
	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	// ModelClass의 프라이빗 변수는 정점 및 인덱스 버퍼이며, 각 버퍼의 크기를 추적하는 두 개의 정수 변수도 포함합니다.
	// DirectX 11의 모든 버퍼는 일반적으로 공통된 ID3D11Buffer 타입을 사용하며, 
	// 처음 생성될 때 버퍼 설명을 통해 그 용도가 명확하게 구분됩니다.
private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;
};

#endif