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
#include "modelclass.h"
#include "lightclass.h"
#include "LightShaderClass.h"
#include "FontClass.h"
#include "FontShaderClass.h"
#include "TextClass.h"
#include "ModelListClass.h"
#include "TimerClass.h"
#include "PositionClass.h"
#include "FrustumClass.h"
#include "ShaderManagerClass.h"


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
    bool Render();
    bool Render(float);
    bool UpdateRenderCountString(int);

private:
    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    ModelClass* m_Model;
    LightClass* m_Light;
    LightShaderClass* m_LightShader;
    FontShaderClass* m_FontShader;
    FontClass* m_Font;
    TextClass* m_RenderCountString;
    ModelListClass* m_ModelList;
    TimerClass* m_Timer;
    PositionClass* m_Position;
    FrustumClass* m_Frustum;
    XMMATRIX m_baseViewMatrix;
};
#endif