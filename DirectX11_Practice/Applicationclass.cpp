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
	char spriteFilename[128];
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

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	// Here we initialize the new sprite object using the sprite_data_01.txt file.
	// Set the sprite info file we will be using.
	strcpy_s(spriteFilename, "../Resource/sprite_data_01.txt");

	// Create and initialize the sprite object.
	m_Sprite = new SpriteClass;

	result = m_Sprite->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, spriteFilename, 50, 50);
	if (!result)
	{
		return false;
	}
	//The new TimerClass object is initialized here.

	// Create and initialize the timer object.
	m_Timer = new TimerClass;

	result = m_Timer->Initialize();
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
	float frameTime;
	bool result;
	
	// Update the system stats.
	m_Timer->Frame();

	// Get the current frame time.
	frameTime = m_Timer->GetTime();

	// Update the sprite object using the frame time.
	m_Sprite->Update(frameTime);

	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;

	//static float rotation = 0.0f;
	//bool result;

	//// Update the rotation variable each frame.
	//rotation -= 0.0174532925f * 0.5f;
	//if (rotation < 0.0f)
	//{
	//	rotation += 360.0f;
	//}
	//// Render the graphics scene.
	//result = Render(rotation);
	//if (!result)
	//{
	//	return false;
	//}

	//return true;
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

	// 장면을 시작하기 위해 버퍼를 초기화합니다.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 2D 렌더링을 위해 D3DClass로부터 직교 투영 행렬(Ortho Matrix)을 가져옵니다.
	// 일반적인 투영 행렬(Projection Matrix) 대신 이 행렬을 전달할 것입니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	
	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	// Put the sprite vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Sprite->Render(m_Direct3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	// Render the sprite with the texture shader.
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Sprite->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Sprite->GetTexture());
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로, 다음 차례의 3D 렌더링을 위해 Z-버퍼를 다시 켭니다.
	m_Direct3D->TurnZBufferOn();

	// 렌더링된 장면을 화면에 출력(Present)합니다.
	m_Direct3D->EndScene();

	return true;
}