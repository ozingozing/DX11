#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "D3DClass.h"
#include "InputClass.h"
#include "CameraClass.h"
#include "ModelClass.h"
#include "LightClass.h"
#include "RenderTextureClass.h"
#include "LightShaderClass.h"
#include "RefractionShaderClass.h"
#include "WaterShaderClass.h"


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;


////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
class ApplicationClass
{
public:
    ApplicationClass();
    ApplicationClass(const ApplicationClass&);
    ~ApplicationClass();

    bool Initialize(int, int, HWND);
    void Shutdown();
    bool Frame(InputClass*);

private:
    bool IsKeyDown(unsigned char key);
    bool Render();
    bool RenderReflectionToTexture();
	bool RenderRefractionToTexture();
    void UpdateCamera(InputClass* Input);

private:
    XMFLOAT3 m_camPos;
    float m_yaw, m_pitch;     // 도(degree) 단위

private:
    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    // 이번 튜토리얼에는 네 가지 서로 다른 모델이 필요합니다.
    // 바닥용 모델, 벽면용 모델, 욕조용 모델, 그리고 물 모델이 그것입니다.
    ModelClass* m_GroundModel, * m_WallModel, * m_BathModel, * m_WaterModel;
    LightClass* m_Light;
    // 또한 반사 텍스처와 굴절 텍스처를 위한
    // '렌더 투 텍스처(render to texture)' 객체 두 개가 필요할 것입니다.
    RenderTextureClass* m_RefractionTexture, * m_ReflectionTexture;
    // 이 튜토리얼에는 라이트 셰이더, 굴절 셰이더, 그리고 물 셰이더가 필요합니다.
    LightShaderClass* m_LightShader;
    RefractionShaderClass* m_RefractionShader;
    WaterShaderClass* m_WaterShader;
    // 또한 물의 위치와 물의 높이를 추적하기 위한 변수 몇 개가 필요합니다.
    float m_waterHeight, m_waterTranslation;

    bool m_flipClip, m_prevF;
};
#endif