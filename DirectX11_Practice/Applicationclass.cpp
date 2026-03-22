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
	char modelFilename[128];
	char textureFilename[128];
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
	m_Camera->SetPosition(0.0f, 2.0f, -12.0f);

	// Create and initialize the model object.
	m_Model = new ModelClass;

	// Set the file anme of the model.
	//strcpy_s(modelFilename, "../Resource/Cube.txt");
	strcpy_s(modelFilename, "../Resource/plane.txt");

	// Set the name of the texture file that we will be loading.
	//C:\Users\sky2503\Desktop\stone01.tga
	strcpy_s(textureFilename, "../Resource/texture01.png");

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// 새로운 광원 셰이더 객체가 여기서 생성되고 초기화됩니다.
	// Create and initialize the light shader object.
	m_LightShader = new LightShaderClass;

	result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Set the number of lights we will use.
	m_numLights = 4;

	// Create and initialize the light objects array.
	m_Lights = new LightClass[m_numLights];

	// Manually set the color and position of each light.
	m_Lights[0].SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red
	m_Lights[0].SetPosition(-3.0f, 1.0f, 3.0f);

	m_Lights[1].SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);  // Green
	m_Lights[1].SetPosition(3.0f, 1.0f, 3.0f);

	m_Lights[2].SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
	m_Lights[2].SetPosition(-3.0f, 1.0f, -3.0f);

	m_Lights[3].SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);  // White
	m_Lights[3].SetPosition(3.0f, 1.0f, -3.0f);

	return true;
}


void ApplicationClass::Shutdown()
{
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
	static float rotation = 0.0f;
	bool result;

	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.5f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}
	// Render the graphics scene.
	result = Render(rotation);
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
bool ApplicationClass::Render(float rotation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMFLOAT4 diffuseColor[4], lightPosition[4];
	int i;
	bool result;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	
	// 네 개의 포인트 라이트에서 두 개의 배열(색상 및 위치)을 설정했습니다.
	// 이렇게 하면 여덟 개의 서로 다른 조명 구성 요소를 보내는 대신
	// 두 개의 배열 변수만 보내면 되므로 훨씬 간편해집니다.
	// 조명의 속성을 가져옵니다.
	for (i = 0; i < m_numLights; i++)
	{
		// 네 가지 광원 색상으로부터 확산 색상 배열을 생성합니다.
		diffuseColor[i] = m_Lights[i].GetDiffuseColor();

		// 네 개의 조명 위치를 이용하여 조명 위치 배열을 생성합니다.
		lightPosition[i] = m_Lights[i].GetPosition();
	}

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// Render the model using the light shader.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(),
		diffuseColor, lightPosition);
	if (!result)
	{
		return false;
	}

	m_Direct3D->EndScene();

	return true;
}