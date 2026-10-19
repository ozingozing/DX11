////////////////////////////////////////////////////////////////////////////////
// 파일명: RenderTextureClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "RenderTextureClass.h"

// 클래스 생성자는 비공개 포인터들을 null로 초기화합니다.
RenderTextureClass::RenderTextureClass()
{
    m_renderTargetTexture = 0;
    m_renderTargetView = 0;
    m_shaderResourceView = 0;
    m_depthStencilBuffer = 0;
    m_depthStencilView = 0;
}

RenderTextureClass::RenderTextureClass(const RenderTextureClass& other)
{
}

RenderTextureClass::~RenderTextureClass()
{
}

// Initialize 함수는 렌더 텍스처 객체를 설정합니다. 이 함수는 먼저 텍스처의 설명을 설정한 다음 해당 텍스처를 생성하여 렌더 타겟 텍스처를 만듭니다.
// 그런 다음 해당 텍스처를 사용하여 렌더 타겟 뷰를 설정하므로, 텍스처에 렌더 타겟으로 드로잉할 수 있게 됩니다.
// 세 번째 작업은 렌더링된 데이터를 호출하는 객체에 제공할 수 있도록 해당 텍스처의 셰이더 리소스 뷰를 생성하는 것입니다.
// 
// 또한 이 함수는 렌더 텍스처의 크기가 다를 수 있으므로 렌더 텍스처의 올바른 원근 렌더링을 위해 투영(Projection) 및 직교(Ortho) 매트릭스를 생성합니다.
// 텍스처가 사용될 곳의 종횡비(가로세로 비율)와 이 렌더 투 텍스처의 종횡비를 항상 일치시켜야 함을 유념하세요. 그렇지 않으면 크기 왜곡이 발생할 수 있습니다.
bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear, int format)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT result;
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    DXGI_FORMAT textureFormat;

    // 텍스처 형식을 설정합니다.
    switch (format)
    {
        case 1:
        {
            textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        default:
        {
            textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }
    }

    // 렌더 텍스처의 너비와 높이를 저장합니다.
    m_textureWidth = textureWidth;
    m_textureHeight = textureHeight;

    // 렌더 타겟 텍스처 설명을 초기화합니다.
    ZeroMemory(&textureDesc, sizeof(textureDesc));

    // 렌더 타겟 텍스처 설명을 설정합니다.
    textureDesc.Width = textureWidth;
    textureDesc.Height = textureHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = textureFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // 렌더 타겟 텍스처를 생성합니다.
    result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
    if (FAILED(result))
    {
        return false;
    }

    // 렌더 타겟 뷰의 설명을 설정합니다.
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    // 렌더 타겟 뷰를 생성합니다.
    result = device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
    if (FAILED(result))
    {
        return false;
    }

    // 셰이더 리소스 뷰의 설명을 설정합니다.
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    // 셰이더 리소스 뷰를 생성합니다.
    result = device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
    if (FAILED(result))
    {
        return false;
    }

    // 깊이 버퍼의 설명을 초기화합니다.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // 깊이 버퍼의 설명을 설정합니다.
    depthBufferDesc.Width = textureWidth;
    depthBufferDesc.Height = textureHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // 작성된 설명을 사용하여 깊이 버퍼용 텍스처를 생성합니다.
    result = device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 깊이 스텐실 뷰 설명을 초기화합니다.
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // 깊이 스텐실 뷰 설명을 설정합니다.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // 깊이 스텐실 뷰를 생성합니다.
    result = device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(result))
    {
        return false;
    }

    // 렌더링을 위한 뷰포트를 설정합니다.
    m_viewport.Width = (float)textureWidth;
    m_viewport.Height = (float)textureHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;

    // 투영 매트릭스를 설정합니다.
    m_projectionMatrix = XMMatrixPerspectiveFovLH((3.141592654f / 4.0f), ((float)textureWidth / (float)textureHeight), screenNear, screenDepth);

    // 2D 렌더링을 위한 직교 투영 매트릭스를 생성합니다.
    m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);

    return true;
}

// Shutdown은 RenderTextureClass가 사용하는 인터페이스들을 해제합니다.
void RenderTextureClass::Shutdown()
{
    if (m_depthStencilView)
    {
        m_depthStencilView->Release();
        m_depthStencilView = 0;
    }

    if (m_depthStencilBuffer)
    {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = 0;
    }

    if (m_shaderResourceView)
    {
        m_shaderResourceView->Release();
        m_shaderResourceView = 0;
    }

    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = 0;
    }

    if (m_renderTargetTexture)
    {
        m_renderTargetTexture->Release();
        m_renderTargetTexture = 0;
    }

    return;
}

// SetRenderTarget 함수는 현재 렌더링 중인 대상 위치를 변경합니다. 
// 보통은 백 버퍼(또는 다른 렌더 텍스처)에 렌더링하지만, 이 함수를 호출하면 이제 이 렌더 텍스처에 렌더링하게 됩니다.
// 이 함수의 호출 전 렌더링하던 곳이나 백 버퍼와 크기가 다를 수 있으므로, 이 렌더 텍스처용 뷰포트도 함께 설정해야 한다는 점에 유의하세요.
void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
    // 렌더 타겟 뷰와 깊이 스텐실 버퍼를 출력 렌더 파이프라인에 바인딩합니다.
    deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // 뷰포트를 설정합니다.
    deviceContext->RSSetViewports(1, &m_viewport);

    return;
}

// ClearRenderTarget 함수는 텍스처와 깊이 버퍼의 이전 내용을 지우기 위해 이 렌더 텍스처에 렌더링하기 직전에 호출됩니다.
void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext* deviceContext, float red, float green, float blue, float alpha)
{
    float color[4];

    // 버퍼를 지울 색상을 설정합니다.
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    // 백 버퍼를 지웁니다. (역주: 렌더 타겟 뷰를 지정된 색상으로 초기화합니다)
    deviceContext->ClearRenderTargetView(m_renderTargetView, color);

    // 깊이 버퍼를 지웁니다.
    deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}

// GetShaderResourceView 함수는 이 렌더 텍스처 객체의 텍스처에 대한 접근 권한을 제공합니다. 
// 렌더 텍스처에 렌더링이 완료되면, 이 텍스처의 데이터는 다른 일반 TextureClass 객체와 마찬가지로 셰이더 텍스처 리소스로 사용할 수 있게 됩니다.
ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView()
{
    return m_shaderResourceView;
}

// 다음 4개의 도우미 함수는 이 렌더 텍스처 객체의 크기, 투영 매트릭스, 직교 매트릭스를 반환합니다.
void RenderTextureClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
    projectionMatrix = m_projectionMatrix;
    return;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
    orthoMatrix = m_orthoMatrix;
    return;
}

int RenderTextureClass::GetTextureWidth()
{
    return m_textureWidth;
}

int RenderTextureClass::GetTextureHeight()
{
    return m_textureHeight;
}