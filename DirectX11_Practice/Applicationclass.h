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
#include "RenderTextureClass.h"
#include "TextureShaderClass.h"
#include "ReflectionShaderClass.h"
#include "ClipPlaneShaderClass.h"


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
    bool RenderReflectionToTexture(float);
    bool Render(float);

private:
    bool m_invertPitch;
    bool m_prevP;                 // 키를 "누른 순간"만 잡기 위한 이전 상태
    bool m_useClip, m_prevC;

    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    ModelClass* m_CubeModel;
    ModelClass* m_FloorModel;
    RenderTextureClass* m_RenderTexture;
    TextureShaderClass* m_TextureShader;
    ReflectionShaderClass* m_ReflectionShader;
	ClipPlaneShaderClass* m_ClipPlaneShader;
};
#endif