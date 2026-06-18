#pragma once

#ifndef _SPECMAPSHADERCLASS_H_
#define _SPECMAPSHADERCLASS_H_


//////////////
// 포함 파일 //
//////////////
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>

using namespace DirectX;
using namespace std;

class SpecMapShaderClass
{
private:
    // 월드, 뷰, 투영 행렬을 저장하는 상수 버퍼 구조체
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    // 조명 정보를 저장하는 상수 버퍼 구조체
    // 확산광, 스페큘러 색상, 스페큘러 강도, 빛의 방향을 셰이더로 전달한다.
    struct LightBufferType
    {
        XMFLOAT4 diffuseColor;
        XMFLOAT4 specularColor;
        float specularPower;
        XMFLOAT3 lightDirection;
    };

    // 카메라 위치 정보를 저장하는 상수 버퍼 구조체
    // 픽셀 셰이더에서 시선 방향을 계산하기 위해 사용한다.
    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;

        // 상수 버퍼는 16바이트 단위 정렬이 필요하므로 패딩 값을 추가한다.
        float padding;
    };

public:
    SpecMapShaderClass();
    SpecMapShaderClass(const SpecMapShaderClass&);
    ~SpecMapShaderClass();

    // 셰이더 클래스를 초기화한다.
    bool Initialize(ID3D11Device*, HWND);

    // 사용한 셰이더 관련 자원을 해제한다.
    void Shutdown();

    // 셰이더를 사용하여 오브젝트를 렌더링한다.
    bool Render(
        ID3D11DeviceContext*,
        int,
        XMMATRIX,
        XMMATRIX,
        XMMATRIX,
        ID3D11ShaderResourceView*,
        ID3D11ShaderResourceView*,
        ID3D11ShaderResourceView*,
        XMFLOAT3,
        XMFLOAT4,
        XMFLOAT3,
        XMFLOAT4,
        float
    );

private:
    // 정점 셰이더와 픽셀 셰이더를 초기화한다.
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);

    // 셰이더 관련 자원을 해제한다.
    void ShutdownShader();

    // 셰이더 컴파일 오류 메시지를 파일로 출력한다.
    void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

    // 렌더링에 필요한 행렬, 텍스처, 카메라, 조명 정보를 셰이더에 전달한다.
    bool SetShaderParameters(
        ID3D11DeviceContext*,
        XMMATRIX,
        XMMATRIX,
        XMMATRIX,
        ID3D11ShaderResourceView*,
        ID3D11ShaderResourceView*,
        ID3D11ShaderResourceView*,
        XMFLOAT3,
        XMFLOAT4,
        XMFLOAT3,
        XMFLOAT4,
        float
    );

    // 셰이더를 적용하고 실제 DrawIndexed를 호출한다.
    void RenderShader(ID3D11DeviceContext*, int);

private:
    // 정점 셰이더 객체
    ID3D11VertexShader* m_vertexShader;

    // 픽셀 셰이더 객체
    ID3D11PixelShader* m_pixelShader;

    // 정점 입력 레이아웃
    ID3D11InputLayout* m_layout;

    // 월드, 뷰, 투영 행렬을 GPU로 전달하기 위한 상수 버퍼
    ID3D11Buffer* m_matrixBuffer;

    // 텍스처 샘플링 방식을 저장하는 샘플러 상태
    ID3D11SamplerState* m_sampleState;

    // 스페큘러 조명 계산에 필요한 조명 정보를 GPU로 전달하기 위한 상수 버퍼
    ID3D11Buffer* m_lightBuffer;

    // 스페큘러 조명 계산에 필요한 카메라 위치 정보를 GPU로 전달하기 위한 상수 버퍼
    ID3D11Buffer* m_cameraBuffer;
};

#endif