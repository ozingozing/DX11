#pragma once
////////////////////////////////////////////////////////////////////////////////
// 파일명: shadermanagerclass.h
// 역할: 애플리케이션에서 사용하는 다중 셰이더들을 중앙 집약적으로 관리하는 클래스
////////////////////////////////////////////////////////////////////////////////
#ifndef _SHADERMANAGERCLASS_H_
#define _SHADERMANAGERCLASS_H_

///////////////////////
//  셰이더 헤더 포함   //
///////////////////////
// 외부 클래스(예: ApplicationClass)에서 개별적으로 포함하지 않도록 
// 매니저 내부에서 모든 셰이더 헤더를 통합 관리합니다.
#include "textureshaderclass.h"
#include "lightshaderclass.h"
#include "normalmapshaderclass.h"
#include "SpecMapShaderClass.h"


////////////////////////////////////////////////////////////////////////////////
// 클래스명: ShaderManagerClass (퍼사드 패턴 적용)
////////////////////////////////////////////////////////////////////////////////
class ShaderManagerClass
{
public:
    ShaderManagerClass();
    ShaderManagerClass(const ShaderManagerClass&);
    ~ShaderManagerClass();

    // 모든 내부 셰이더 객체들을 생성 및 초기화합니다.
    bool Initialize(ID3D11Device* device, HWND hwnd);

    // 사용된 모든 셰이더 자원을 일괄 해제합니다.
    void Shutdown();

    /////////////////////////////////////////
    // 개별 셰이더 전용 렌더링 인터페이스       //
    /////////////////////////////////////////
    // 외부에서는 이 인터페이스들을 통해 복잡한 과정 없이 함수 호출 하나로 원하는 셰이더를 렌더링합니다.

    // 1. 텍스처 셰이더를 사용한 렌더링
    bool RenderTextureShader(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
        ID3D11ShaderResourceView* texture);

    // 2. 라이트 셰이더를 사용한 렌더링 (텍스처 + 디퓨즈 라이트)
    bool RenderLightShader(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
        ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor);

    // 3. 노멀 맵 셰이더를 사용한 렌더링 (텍스처 + 노멀 맵 + 디퓨즈 라이트)
    bool RenderNormalMapShader(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
        ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalMap,
        XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor);

    bool RenderSpecMapShader(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
        ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3,
		XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower);

private:
    /////////////////////////////////////////
    //     내부 셰이더 객체 포인터            //
    /////////////////////////////////////////
    // 매니저 클래스가 캡슐화하여 내부에서 관리하는 개별 셰이더 객체들입니다.
    TextureShaderClass* m_TextureShader;
    LightShaderClass* m_LightShader;
    NormalMapShaderClass* m_NormalMapShader;
	SpecMapShaderClass* m_SpecMapShader;
};

#endif