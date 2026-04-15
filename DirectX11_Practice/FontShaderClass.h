////////////////////////////////////////////////////////////////////////////////
// 파일명: fontshaderclass.h
// 목적: 폰트 전용 셰이더를 관리하고 렌더링 파이프라인을 제어하는 클래스
////////////////////////////////////////////////////////////////////////////////
#ifndef _FONTSHADERCLASS_H_
#define _FONTSHADERCLASS_H_

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
// Class name: FontShaderClass
////////////////////////////////////////////////////////////////////////////////
class FontShaderClass
{
private:
    // 정점 셰이더의 MatrixBuffer와 매칭되는 구조체입니다.
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    // [추가된 구조체] 픽셀 셰이더의 PixelBuffer와 매칭되는 구조체입니다.
    // 렌더링될 텍스트의 색상 정보(RGBA)만 포함합니다.
    struct PixelBufferType
    {
        XMFLOAT4 pixelColor;
    };

public:
    FontShaderClass();
    FontShaderClass(const FontShaderClass&);
    ~FontShaderClass();

    // 셰이더 초기화 및 해제
    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();

    // 셰이더 매개변수 설정 및 실제 렌더링 수행
    bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);

private:
    // 내부 헬퍼 함수들
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    // 상수 버퍼 및 리소스 설정
    bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader; // 정점 셰이더 객체
    ID3D11PixelShader* m_pixelShader;   // 픽셀 셰이더 객체
    ID3D11InputLayout* m_layout;        // 입력 레이아웃
    ID3D11Buffer* m_matrixBuffer;       // 월드/뷰/투영 행렬용 상수 버퍼
    ID3D11SamplerState* m_sampleState;  // 텍스처 샘플러 상태

    // [추가된 버퍼] 텍스트 폰트의 색상을 결정하는 픽셀 컬러용 상수 버퍼입니다.
    ID3D11Buffer* m_pixelBuffer;
};

#endif