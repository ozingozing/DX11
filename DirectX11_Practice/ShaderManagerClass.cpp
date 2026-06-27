////////////////////////////////////////////////////////////////////////////////
// 파일명: shadermanagerclass.cpp
// 역할: ShaderManagerClass의 멤버 함수들을 구현하며, 개별 셰이더의 생명주기와
//       렌더링 파이프라인 호출을 중앙 제어합니다.
////////////////////////////////////////////////////////////////////////////////
#include "ShaderManagerClass.h"

//------------------------------------------------------------------------------
// 생성자: 객체가 생성될 때 모든 셰이더 포인터를 안전하게 NULL(0)로 초기화합니다.
//------------------------------------------------------------------------------
ShaderManagerClass::ShaderManagerClass()
{
    m_TextureShader = 0;
    m_LightShader = 0;
    m_NormalMapShader = 0;
}


ShaderManagerClass::ShaderManagerClass(const ShaderManagerClass& other)
{
}


ShaderManagerClass::~ShaderManagerClass()
{
}


//------------------------------------------------------------------------------
// Initialize: 애플리케이션 시작 시 관리할 모든 셰이더 객체들을 동적 할당하고 초기화합니다.
//------------------------------------------------------------------------------
bool ShaderManagerClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    // 1. 텍스처 셰이더 객체 생성 및 초기화
    m_TextureShader = new TextureShaderClass;

    result = m_TextureShader->Initialize(device, hwnd);
    if (!result)
    {
        return false;
    }

    // 2. 라이트 셰이더 객체 생성 및 초기화
    m_LightShader = new LightShaderClass;

    result = m_LightShader->Initialize(device, hwnd);
    if (!result)
    {
        return false;
    }

    // 3. 노멀 맵 셰이더 객체 생성 및 초기화
    m_NormalMapShader = new NormalMapShaderClass;

    result = m_NormalMapShader->Initialize(device, hwnd);
    if (!result)
    {
        return false;
    }

	// 4. 스페큘러 맵 셰이더 객체 생성 및 초기화
	m_SpecMapShader = new SpecMapShaderClass;
	result = m_SpecMapShader->Initialize(device, hwnd);
    if (!result)
    {
        return false;
	}

    return true;
}


//------------------------------------------------------------------------------
// Shutdown: 애플리케이션 종료 시 생성된 역순으로 모든 셰이더 자원을 일괄 해제합니다.
//------------------------------------------------------------------------------
void ShaderManagerClass::Shutdown()
{
    // 노멀 맵 셰이더 자원 해제 및 메모리 반환
    if (m_NormalMapShader)
    {
        m_NormalMapShader->Shutdown();
        delete m_NormalMapShader;
        m_NormalMapShader = 0;
    }

	// 스페큘러 맵 셰이더 자원 해제 및 메모리 반환
	if (m_SpecMapShader)
	{
		m_SpecMapShader->Shutdown();
		delete m_SpecMapShader;
		m_SpecMapShader = 0;
	}

    // 라이트 셰이더 자원 해제 및 메모리 반환
    if (m_LightShader)
    {
        m_LightShader->Shutdown();
        delete m_LightShader;
        m_LightShader = 0;
    }

    // 텍스처 셰이더 자원 해제 및 메모리 반환
    if (m_TextureShader)
    {
        m_TextureShader->Shutdown();
        delete m_TextureShader;
        m_TextureShader = 0;
    }

    return;
}


//==============================================================================
// 개별 셰이더 렌더링 호출 함수 인터페이스들
// 외부(ApplicationClass)에서는 이 매니저 객체의 포인터만 들고 있으면, 
// 하위 셰이더들의 복잡한 파라미터를 신경 쓸 필요 없이 아래 함수들로 일괄 렌더링이 가능합니다.
//==============================================================================

// 1. 기본 텍스처 매핑 렌더링 함수
bool ShaderManagerClass::RenderTextureShader(ID3D11DeviceContext* deviceContext, int indexCount,
    XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture)
{
    bool result;

    // 내부 텍스처 셰이더 객체에 렌더링 명령 위임
    result = m_TextureShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture);
    if (!result)
    {
        return false;
    }

    return true;
}


// 2. 라이팅(디퓨즈) 매핑 렌더링 함수
bool ShaderManagerClass::RenderLightShader(ID3D11DeviceContext* deviceContext, int indexCount,
    XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
    bool result;

    // 내부 라이트 셰이더 객체에 렌더링 명령 위임
    result = m_LightShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, diffuseColor);
    if (!result)
    {
        return false;
    }

    return true;
}


// 3. 노멀 매핑(입체감 추가) 렌더링 함수
bool ShaderManagerClass::RenderNormalMapShader(ID3D11DeviceContext* deviceContext, int indexCount,
    XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture,
    XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
    bool result;

    // 내부 노멀 맵 셰이더 객체에 렌더링 명령 위임 (색상 텍스처와 노멀 텍스처 2개를 전달)
    result = m_NormalMapShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDirection, diffuseColor);
    if (!result)
    {
        return false;
    }

    return true;
}

bool ShaderManagerClass::RenderSpecMapShader(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    bool result;
    // 내부 스페큘러 맵 셰이더 객체에 렌더링 명령 위임 (색상 텍스처, 노멀 텍스처, 스페큘러 텍스처 3개를 전달)
    result = m_SpecMapShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture1, texture2, texture3, lightDirection, diffuseColor, cameraPosition, specularColor, specularPower);
    if (!result)
    {
        return false;
    }
	return true;
}