////////////////////////////////////////////////////////////////////////////////
// Filename: ApplicationClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ApplicationClass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_cameraZ = 0;
	m_ClipPlaneShader = 0;
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
	char modelFilename[128], textureFilename1[128], renderString[32];
	bool result;


	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// "평소와 마찬가지로 카메라 객체를 생성합니다. 하지만 카메라가 회전할 때마다 뷰(View) 매트릭스가 수정되므로,
	// 시작할 때 뷰 매트릭스의 복사본을 미리 만들어 둡니다.
	// 매 프레임마다 동일한 위치에 텍스트를 렌더링하려면 이 수정되지 않은 뷰 매트릭스가 필요합니다."
	m_Camera = new CameraClass;

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	// Set the file name of the model.
	strcpy_s(modelFilename, "../Resource/cube.txt");
	// Set the file name of the textures.
	strcpy_s(textureFilename1, "../Resource/stone01.tga");


	// Create and initialize the model object.
	m_Model = new ModelClass;

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		return false;
	}

	// Create and initialize the clip plane shader object.
	m_ClipPlaneShader = new ClipPlaneShaderClass;

	result = m_ClipPlaneShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the clip plane shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the clip plane shader object.
	if (m_ClipPlaneShader)
	{
		m_ClipPlaneShader->Shutdown();
		delete m_ClipPlaneShader;
		m_ClipPlaneShader = 0;
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


bool ApplicationClass::Frame(InputClass* Input)
{
	static float rotation = 0.0f;
	const float moveSpeed = 0.05f;
	bool result;


	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}

	// 앞(위 화살표) / 뒤(아래 화살표)
	if (Input->IsUpArrowPressed())   m_cameraZ += moveSpeed;   // +z = 큐브 쪽으로
	if (Input->IsDownArrowPressed()) m_cameraZ -= moveSpeed;

	if (m_cameraZ > -2.0f) m_cameraZ = -2.0f; // 카메라가 큐브를 뚫고 들어가지 않도록 제한
	if (m_cameraZ < -30.0f) m_cameraZ = -30.0f; // 카메라가 너무 멀리 가지 않도록 제한

	m_Camera->SetPosition(0.0f, 0.0f, m_cameraZ);
	m_Camera->Render();

	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.25f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	// Render the final graphics scene.
	result = Render(rotation);
	if (!result)
	{
		return false;
	}

	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	float positionX, positionY, positionZ, radius;
	int modelCount, renderCount, i;
	bool renderModel, result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

bool ApplicationClass::Render(float rotation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMFLOAT4 clipPlane;
	bool result;

	clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// Rotate the world matrix by the rotation value so that the cube will spin.
	worldMatrix = XMMatrixRotationY(rotation);

	// Render the model using the texture shader.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_ClipPlaneShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(), clipPlane);
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}