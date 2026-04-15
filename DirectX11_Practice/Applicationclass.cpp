////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_LightShader = 0;
	m_Lights = 0;
	m_TextureShader = 0;

	m_Sprite = 0;
	m_Timer = 0;

	m_FontShader = 0;
	m_Font = 0;
	m_TextString1 = 0;
	m_TextString2 = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}

// The model initialization now takes
// in the filename of the model file it is loading.
// In this tutorial we will use the cube.txt file
// so this model loads in a 3D cube object for rendering.
bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char testString1[32], testString2[32];
	bool result;


	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	// 텍스트 렌더링을 위한 새로운 FontShaderClass 객체를 초기화합니다.

	// 폰트 셰이더 객체를 생성하고 초기화합니다.
	m_FontShader = new FontShaderClass;

	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"폰트 셰이더 객체를 초기화할 수 없습니다.", L"에러", MB_OK);
		return false;
	}

	// TextClass가 사용할 문장을 구축할 FontClass를 초기화합니다.

	// 폰트 객체를 생성하고 초기화합니다.
	m_Font = new FontClass;

	result = m_Font->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), 0);
	if (!result)
	{
		return false;
	}

	// 화면에 출력할 두 개의 문장을 설정합니다.

	// 표시할 문자열 내용을 복사합니다.
	strcpy_s(testString1, "Hello");
	strcpy_s(testString2, "Goodbye");

	// 위에서 생성한 두 개의 문자열을 렌더링하기 위해 두 개의 TextClass 객체를 생성합니다.
	// 첫 번째 문장은 초록색(0, 1, 0)으로 설정하여 좌표 (10, 10)에 렌더링하고,
	// 두 번째 문장은 노란색(1, 1, 0)으로 설정하여 좌표 (10, 50)에 렌더링합니다.

	// 첫 번째 텍스트 객체를 생성하고 초기화합니다.
	m_TextString1 = new TextClass;

	result = m_TextString1->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, 32, m_Font, testString1, 10, 10, 0.0f, 1.0f, 0.0f);
	if (!result)
	{
		return false;
	}

	// 두 번째 텍스트 객체를 생성하고 초기화합니다.
	m_TextString2 = new TextClass;

	result = m_TextString2->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, 32, m_Font, testString2, 10, 50, 1.0f, 1.0f, 0.0f);
	if (!result)
	{
		return false;
	}

	return true;


	//char modelFilename[128];
	//char textureFilename[128];
	//bool result;


	//// Create and initialize the Direct3D object.
	//m_Direct3D = new D3DClass;

	//result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	//if (!result)
	//{
	//	MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
	//	return false;
	//}

	//// Create the camera object.
	//m_Camera = new CameraClass;

	//// Set the initial position of the camera.
	//m_Camera->SetPosition(0.0f, 2.0f, -12.0f);

	//// Create and initialize the model object.
	//m_Model = new ModelClass;

	//// Set the file anme of the model.
	////strcpy_s(modelFilename, "../Resource/Cube.txt");
	//strcpy_s(modelFilename, "../Resource/plane.txt");

	//// Set the name of the texture file that we will be loading.
	////C:\Users\sky2503\Desktop\stone01.tga
	//strcpy_s(textureFilename, "../Resource/texture01.png");

	//result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename);
	//if (!result)
	//{
	//	MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
	//	return false;
	//}

	//// 새로운 광원 셰이더 객체가 여기서 생성되고 초기화됩니다.
	//// Create and initialize the light shader object.
	//m_LightShader = new LightShaderClass;

	//result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	//if (!result)
	//{
	//	MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
	//	return false;
	//}

	//// Set the number of lights we will use.
	//m_numLights = 4;

	//// Create and initialize the light objects array.
	//m_Lights = new LightClass[m_numLights];

	//// Manually set the color and position of each light.
	//m_Lights[0].SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red
	//m_Lights[0].SetPosition(-3.0f, 1.0f, 3.0f);

	//m_Lights[1].SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);  // Green
	//m_Lights[1].SetPosition(3.0f, 1.0f, 3.0f);

	//m_Lights[2].SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
	//m_Lights[2].SetPosition(-3.0f, 1.0f, -3.0f);

	//m_Lights[3].SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);  // White
	//m_Lights[3].SetPosition(3.0f, 1.0f, -3.0f);

	//return true;
}


void ApplicationClass::Shutdown()
{
	//Release the two text objects.
	// Release the text string objects.
	if (m_TextString2)
	{
		m_TextString2->Shutdown();
		delete m_TextString2;
		m_TextString2 = 0;
	}

	if (m_TextString1)
	{
		m_TextString1->Shutdown();
		delete m_TextString1;
		m_TextString1 = 0;
	}

	// Release the Font and the FontShader objects.
	// Release the font object.
	if (m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = 0;
	}

	// Release the font shader object.
	if (m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = 0;
	}

	// Release the timer object.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the sprite object.
	if (m_Sprite)
	{
		m_Sprite->Shutdown();
		delete m_Sprite;
		m_Sprite = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the light objects.
	if (m_Lights)
	{
		delete[] m_Lights;
		m_Lights = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result;


	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

// Render 함수는 전체 렌더링 과정을 제어하며, 이번 단계에서 가장 많은 변경이 이루어졌습니다.
// 1. 장면 초기화: 먼저 장면을 검은색(Black)으로 깨끗하게 지우는 것으로 시작합니다.
// 2. 뷰 행렬 생성: 카메라 객체의 Render 함수를 호출하여, 초기화 시 설정된 위치 정보를 바탕으로 뷰 행렬을 생성합니다.
// 3. 행렬 정보 취득: 생성된 뷰 행렬의 복사본을 카메라 클래스에서 가져오고, D3DClass 객체로부터 월드 행렬과 투영 행렬의 복사본을 각각 가져옵니다.

// 4. 모델 준비: ModelClass::Render 함수를 호출하여 초록색 삼각형의 기하학적 구조(정점 및 인덱스 데이터)를 그래픽 파이프라인에 배치합니다.
// 5. 셰이더 렌더링: 정점들이 준비되면 컬러 셰이더를 호출합니다. 이때 모델 정보와 각 정점의 위치를 결정할 세 가지 행렬(World, View, Projection)을 전달하여 실제 그리기를 수행합니다.
// 6. 출력: 초록색 삼각형이 백 버퍼(Back Buffer)에 그려지면, EndScene을 호출하여 완성된 장면을 실제 화면에 표시합니다.
bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	// 장면을 시작하기 위해 버퍼를 비웁니다 (검은색으로 초기화).
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라 및 D3D 객체로부터 월드, 뷰, 직교 투영(Ortho) 행렬을 가져옵니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	// 텍스트를 렌더링하기 전에 Z 버퍼(깊이 시험)를 꺼야 합니다. 
	// 그래야 깊이 정보에 상관없이 텍스트가 화면 최상단에 그려집니다.
	// 두 번째로, 알파 블렌딩을 활성화하여 글자 주변의 검은색 배경은 버리고 
	// 실제 글자 픽셀만 그려지도록 합니다. 만약 알파 블렌딩을 끄고 텍스트를 그리면
	// 글자 주변에 원치 않는 검은색 상자가 나타나게 됩니다.

	// 2D 렌더링을 위해 Z 버퍼를 끄고 알파 블렌딩을 활성화합니다.
	m_Direct3D->TurnZBufferOff();
	m_Direct3D->EnableAlphaBlending();

	// 폰트와 폰트 셰이더를 사용하여 첫 번째 텍스트 문자열을 렌더링합니다.

	// 첫 번째 텍스트 문자열의 버퍼를 파이프라인에 설정합니다.
	m_TextString1->Render(m_Direct3D->GetDeviceContext());

	// 폰트 셰이더를 통해 실제로 화면에 그립니다.
	result = m_FontShader->Render(m_Direct3D->GetDeviceContext(), m_TextString1->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_Font->GetTexture(), m_TextString1->GetPixelColor());
	if (!result)
	{
		return false;
	}

	// 폰트와 폰트 셰이더를 사용하여 두 번째 텍스트 문자열을 렌더링합니다.

	// 두 번째 텍스트 문자열의 버퍼를 파이프라인에 설정합니다.
	m_TextString2->Render(m_Direct3D->GetDeviceContext());

	// 폰트 셰이더를 통해 실제로 화면에 그립니다.
	result = m_FontShader->Render(m_Direct3D->GetDeviceContext(), m_TextString2->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_Font->GetTexture(), m_TextString2->GetPixelColor());
	if (!result)
	{
		return false;
	}

	// 텍스트 렌더링이 끝났으므로, 이후의 3D 렌더링이 정상적으로 출력되도록 
	// Z 버퍼를 다시 켜고 알파 블렌딩을 비활성화합니다.

	// 2D 렌더링 완료 후 Z 버퍼를 켜고 알파 블렌딩을 비활성화합니다.
	m_Direct3D->TurnZBufferOn();
	m_Direct3D->DisableAlphaBlending();

	// 렌더링된 장면을 화면에 표시합니다.
	m_Direct3D->EndScene();

	return true;
}