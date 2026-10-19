#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
class RenderTextureClass
{
public:
    RenderTextureClass();
    RenderTextureClass(const RenderTextureClass&);
    ~RenderTextureClass();

    bool Initialize(ID3D11Device* device, int width, int height,
        float screenDepth, float screenNear, int format);
    void Shutdown();

    void SetRenderTarget(ID3D11DeviceContext* context);      // 그리기 목적지를 이 텍스처로
    void ClearRenderTarget(ID3D11DeviceContext* context,
        float r, float g, float b, float a);
    ID3D11ShaderResourceView* GetShaderResourceView();       // 결과를 텍스처로 꺼내기

    void GetProjectionMatrix(XMMATRIX& out);                 // 이 텍스처 크기에 맞춘 투영 행렬
    void GetOrthoMatrix(XMMATRIX& out);
    int GetTextureWidth();
    int GetTextureHeight();

private:
    int m_textureWidth, m_textureHeight;
    ID3D11Texture2D* m_renderTargetTexture;             // 색상 텍스처
    ID3D11RenderTargetView* m_renderTargetView;         // 쓰기용 뷰
    ID3D11ShaderResourceView* m_shaderResourceView;     // 읽기용 뷰
    ID3D11Texture2D* m_depthStencilBuffer;              // 전용 깊이 버퍼
    ID3D11DepthStencilView* m_depthStencilView;         // 깊이 스텐실 뷰
    D3D11_VIEWPORT            m_viewport;               // 텍스처 크기의 뷰포트
    XMMATRIX m_projectionMatrix, m_orthoMatrix;
};

#endif