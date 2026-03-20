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

    // 정점 셰이더의 새로운 카메라 상수 버퍼와 일치하는 새로운 카메라 버퍼 구조체를 추가합니다.
    // 이 구조체에 sizeof를 사용할 때 CreateBuffer가 실패하는 것을 방지하기 위해
    // 구조체 크기가 16의 배수가 되도록 패딩을 추가합니다.
    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;
        float padding;
    };

    /**
     * [LightBufferType 구조체 수정 사항]
     * * 1. 픽셀 셰이더의 광원 상수 버퍼와 일치하도록 반사광 색상(SpecularColor)과
     * 반사광 강도(SpecularPower) 필드를 추가함.
     * * 2. 16바이트 정렬 규칙(Packing Rule) 준수:
     * - 상수 버퍼는 각 슬롯(float4 단위)이 논리적으로 16바이트 경계에 맞춰져야 함.
     * - 구조체 크기만 16의 배수라고 해서 해결되는 것이 아니라, 데이터 배치 순서가 중요함.
     * * 3. 최적화된 배치:
     * - '반사광 강도(float)'를 '광원 방향(float3)' 바로 옆에 배치하여
     * 하나의 float4 슬롯을 완성함 (패딩 공간을 데이터로 활용).
     * - 만약 강도 값을 맨 마지막에 두고 광원 방향 뒤를 비워두었다면,
     * 정렬 오류로 인해 셰이더가 데이터를 잘못 읽어 빌드/실행 오류가 발생할 수 있음.
     */
    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;   // 확산광 색상
        XMFLOAT3 lightDirection; // 광원 방향
        float specularPower;
        XMFLOAT4 specularColor;
    };

public:
    LightShaderClass();
    LightShaderClass(const LightShaderClass&);
    ~LightShaderClass();

    // 초기화 및 해제
    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();

    // 렌더링 실행: 행렬, 텍스처, 조명 방향 및 색상을 인자로 받음
    bool Render(ID3D11DeviceContext*, int,
        XMMATRIX, XMMATRIX, XMMATRIX,
        ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4,
        XMFLOAT4, XMFLOAT3, XMFLOAT4, float);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    // 실제 GPU 버퍼에 값을 쓰는 내부 함수
    bool SetShaderParameters(ID3D11DeviceContext*,
        XMMATRIX, XMMATRIX, XMMATRIX,
        ID3D11ShaderResourceView*, XMFLOAT3,
        XMFLOAT4, XMFLOAT4, XMFLOAT3, XMFLOAT4, float);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer; // 행렬 상수 버퍼
    // 여기서는 정점 셰이더에서 카메라 위치를 설정하는 데 사용될
    // 새로운 카메라 상수 버퍼를 추가합니다.
    ID3D11Buffer* m_cameraBuffer;
    ID3D11Buffer* m_lightBuffer;  // 조명 상수 버퍼 (추가됨)
};

#endif