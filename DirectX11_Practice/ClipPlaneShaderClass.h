#pragma once
////////////////////////////////////////////////////////////////////////////////
// 파일명: clipplaneshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CLIPPLANESHADERCLASS_H_
#define _CLIPPLANESHADERCLASS_H_


//////////////
// 포함 파일 //
//////////////
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// 클래스 이름: ClipPlaneShaderClass
////////////////////////////////////////////////////////////////////////////////
class ClipPlaneShaderClass
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    // 버텍스 셰이더의 클립 평면 상수 버퍼와 일치하는 새로운 구조체입니다.
    struct ClipPlaneBufferType
    {
        XMFLOAT4 clipPlane;
    };

public:
    ClipPlaneShaderClass();
    ClipPlaneShaderClass(const ClipPlaneShaderClass&);
    ~ClipPlaneShaderClass();

    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11SamplerState* m_sampleState;

    // 클립 평면 데이터를 위한 새로운 상수 버퍼입니다.
    ID3D11Buffer* m_clipPlaneBuffer;
};

#endif