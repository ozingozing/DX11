////////////////////////////////////////////////////////////////////////////////
// Filename: lightshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_


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
// Class name: LightShaderClass
////////////////////////////////////////////////////////////////////////////////
class LightShaderClass
{
private:
    // 정점 셰이더용 상수 버퍼 구조체
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    // 픽셀 셰이더용 조명 상수 버퍼 구조체
    struct LightBufferType
    {
        XMFLOAT4 diffuseColor;   // 확산광 색상
        XMFLOAT3 lightDirection; // 광원 방향
        float padding;           // 16바이트 정렬을 위한 패딩 (구조체 크기 맞춰주기용)
        XMFLOAT4 ambientColor;
    };

public:
    LightShaderClass();
    LightShaderClass(const LightShaderClass&);
    ~LightShaderClass();

    // 초기화 및 해제
    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();

    // 렌더링 실행: 행렬, 텍스처, 조명 방향 및 색상을 인자로 받음
    bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX,
        ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    // 실제 GPU 버퍼에 값을 쓰는 내부 함수
    bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX,
        ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer; // 행렬 상수 버퍼
    ID3D11Buffer* m_lightBuffer;  // 조명 상수 버퍼 (추가됨)
};

#endif