////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_ColorShader = 0;
	m_TextureShader = 0;
	m_LightShader = 0;
	m_Light = 0;
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
	m_Camera->SetPosition(0.0f, 50.f, -300.0f);

	// Create and initialize the model object.
	m_Model = new ModelClass;

	// Set the file anme of the model.
	//strcpy_s(modelFilename, "../Resource/Cube.txt");
	strcpy_s(modelFilename, "../Resource/Man.fbx");

	// Set the name of the texture file that we will be loading.
	//C:\Users\sky2503\Desktop\stone01.tga
	strcpy_s(textureFilename, "../Resource/texture01.png");

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
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

	// 새로운 광원 객체가 여기에 생성됩니다.
	// 광원의 색상은 흰색으로 설정되고
	// 광원의 방향은 양의 Z축을 따라 아래쪽으로 설정됩니다.
	// Create and initialize the light object.
	m_Light = new LightClass;

	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the light object.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}
	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the color shader object.
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, rotateMatrix, translateMatrix, scaleMatrix, srMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 1. Y축 기준 회전 행렬 생성 (rotation 변수 사용)
	rotateMatrix = XMMatrixRotationY(rotation);

	// 2. 이동 행렬 생성 (큐브를 왼쪽으로 2유닛 이동)
	translateMatrix = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);

	// 3. 두 행렬을 곱하여 최종 월드 변환 행렬 생성 (회전 -> 이동 순서)
	worldMatrix = XMMatrixMultiply(rotateMatrix, translateMatrix);

	// 모델의 정점(Vertex) 및 인덱스(Index) 버퍼를 그래픽 파이프라인에 설정하여 그릴 준비를 합니다.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// 라이트 쉐이더를 사용하여 모델을 렌더링합니다.
	// 월드, 뷰, 투영 행렬과 함께 텍스처, 조명 방향, 조명 색상 정보를 전달합니다.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(),
		m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	// 1. 크기 조절 행렬 생성 (모든 축에 대해 0.5배로 균등하게 축소)
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	// 2. 회전 행렬 생성 (첫 번째 큐브와 동일한 Y축 회전 적용)
	rotateMatrix = XMMatrixRotationY(rotation);

	// 3. 이동 행렬 생성 (반대 방향인 오른쪽으로 20유닛 이동)
	translateMatrix = XMMatrixTranslation(20.f, 0.0f, 0.0f);

	// 4. SRT 순서(Scale -> Rotate -> Translate)에 맞춰 행렬을 결합합니다.
	// 먼저 크기 조절과 회전을 곱합니다.
	srMatrix = XMMatrixMultiply(scaleMatrix, rotateMatrix);
	// 그 결과에 이동 행렬을 곱하여 최종 월드 변환 행렬을 완성합니다.
	worldMatrix = XMMatrixMultiply(srMatrix, translateMatrix);

	// 모델 버퍼를 다시 준비합니다.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// 새로운 월드 행렬을 적용하여 두 번째 큐브를 화면에 렌더링합니다.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(),
		m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	//// Rotate the world matrix by the rotation value so that the triangle will spin.
	//worldMatrix = XMMatrixRotationY(rotation);

	//// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//m_Model->Render(m_Direct3D->GetDeviceContext());

	//// Render the model using the light shader.
	//result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(),
	//	m_Light->GetDirection(), m_Light->GetDiffuseColor());
	//if (!result)
	//{
	//	return false;
	//}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}