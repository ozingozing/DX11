#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class TextureShaderClass
{
private:
	// 셰이더에 전달될 행렬 데이터를 위한 구조체입니다.
	struct MatrixBufferType
	{
		XMMATRIX world;      // 월드 행렬
		XMMATRIX view;       // 뷰 행렬
		XMMATRIX projection; // 투영 행렬
	};

public:
	// 생성자, 복사 생성자, 소멸자
	TextureShaderClass();
	TextureShaderClass(const TextureShaderClass&);
	~TextureShaderClass();

	// 셰이더를 초기화하는 함수
	bool Initialize(ID3D11Device*, HWND);
	// 셰이더 관련 리소스를 해제하는 함수
	void Shutdown();
	// 모델의 텍스처를 렌더링하는 함수
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*);

private:
	// 셰이더 파일을 초기화하는 함수
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	// 셰이더를 종료하는 함수
	void ShutdownShader();
	// 셰이더 컴파일 오류 메시지를 출력하는 함수
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	// 셰이더에 매개변수를 설정하는 함수
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*);
	// 셰이더를 사용하여 렌더링하는 함수
	void RenderShader(ID3D11DeviceContext*, int);

private:
	// 셰이더 객체에 대한 포인터들
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	// 샘플러 상태 포인터를 위한 새로운 private 변수입니다.
	// 이 포인터는 텍스처 셰이더와 상호 작용하는 데 사용됩니다.
	ID3D11SamplerState* m_sampleState;
};
#endif