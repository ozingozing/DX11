#pragma once
////////////////////////////////////////////////////////////////////////////////
// 파일명: FogShaderClass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FOGSHADERCLASS_H_
#define _FOGSHADERCLASS_H_


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
// 클래스 이름: FogShaderClass
////////////////////////////////////////////////////////////////////////////////
class FogShaderClass
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    // 안개(fog) 버퍼 데이터를 위한 새로운 구조체입니다. 16바이트의 배수를 맞추기 위해 8바이트의 패딩(padding)이 추가되었습니다.
    struct FogBufferType
    {
        float fogStart;
        float fogEnd;
        float padding1, padding2;
    };

public:
    FogShaderClass();
    FogShaderClass(const FogShaderClass&);
    ~FogShaderClass();

    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, float, float);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, float, float);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11Buffer* m_matrixBuffer;
    ID3D11SamplerState* m_sampleState;

    // 셰이더에서 안개의 시작 및 끝 위치를 설정하는 데 사용할 안개 데이터용 새 버퍼입니다.
    ID3D11Buffer* m_fogBuffer;
};

#endif