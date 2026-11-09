////////////////////////////////////////////////////////////////////////////////
// Filename: ApplicationClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ApplicationClass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_WindowModel = 0;
	m_FireShader = 0;
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
	char modelFilename[128], textureFilename1[128], textureFilename2[128], textureFilename3[128];
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

	m_Camera->SetPosition(-4.0f, 5.0f, -4.0f);
	m_Camera->SetRotation(5.f, 45.0f, 0.0f);
	m_camPos = XMFLOAT3(-4.0f, 5.0f, -4.0f);
	m_yaw = 45.0f;
	m_pitch = 15.0f;

	// Set the file name of the model.
	strcpy_s(modelFilename, "../Resource/square.txt");

	// Set the file name of the textures for the model.
	strcpy_s(textureFilename1, "../Resource/Fire01.tga");
	strcpy_s(textureFilename2, "../Resource/FireNoise01.tga");
	strcpy_s(textureFilename3, "../Resource/FireAlpha01.tga");

	// Create and initialize the model object.
	m_Model = new ModelClass;

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1, textureFilename2, textureFilename3);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the fire shader object.
	m_FireShader = new FireShaderClass;

	result = m_FireShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the fire shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the fire shader object.
	if (m_FireShader)
	{
		m_FireShader->Shutdown();
		delete m_FireShader;
		m_FireShader = 0;
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
	static float glassTranslation = 0.0f;
	const float moveSpeed = 0.05f;
	bool result;

	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}

	UpdateCamera(Input);


	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.25f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	glassTranslation += 0.002f;
	if (glassTranslation > 1.0f)
	{
		glassTranslation -= 1.0f;
	}

	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}
	return true;
}

void ApplicationClass::UpdateCamera(InputClass* Input)
{
	const float lookSpeed = 0.1f;
	const float moveSpeed = Input->IsKeyDown(DIK_LSHIFT) ? 0.3f : 0.1f;

	// 1.마우스 -> 각도
	int dx, dy;
	Input->GetMouseDelta(dx, dy);
	m_yaw += dx * lookSpeed;
	m_pitch += dy * lookSpeed;
	if (m_pitch > 89.0f) m_pitch = 89.0f;
	if (m_pitch < -89.0f) m_pitch = -89.0f;
	if (m_yaw > 360.0f) m_yaw -= 360.0f;
	if (m_yaw < 0.0f) m_yaw += 360.0f;

	// 2.각도 -> 앞/오른쪽 방향
	float yaw = XMConvertToRadians(m_yaw);
	float pitch = XMConvertToRadians(m_pitch);
	XMFLOAT3 foward(cosf(pitch) * sinf(yaw), -sinf(pitch), cosf(pitch) * cosf(yaw));
	XMFLOAT3 right(cosf(yaw), 0.0f, -sinf(yaw));

	// 3.키보드 -> 이동
	if (Input->IsKeyDown(DIK_W)) m_camPos.x += foward.x * moveSpeed, m_camPos.y += foward.y * moveSpeed, m_camPos.z += foward.z * moveSpeed;
	if (Input->IsKeyDown(DIK_S)) m_camPos.x -= foward.x * moveSpeed, m_camPos.y -= foward.y * moveSpeed, m_camPos.z -= foward.z * moveSpeed;
	if (Input->IsKeyDown(DIK_A)) m_camPos.x -= right.x * moveSpeed, m_camPos.z -= right.z * moveSpeed;
	if (Input->IsKeyDown(DIK_D)) m_camPos.x += right.x * moveSpeed, m_camPos.z += right.z * moveSpeed;
	if (Input->IsKeyDown(DIK_SPACE)) m_camPos.y += moveSpeed;
	if (Input->IsKeyDown(DIK_LCONTROL)) m_camPos.y -= moveSpeed;

	// 5. 카메라에 적용
	m_Camera->SetPosition(m_camPos.x, m_camPos.y, m_camPos.z);
	m_Camera->SetRotation(m_pitch, m_yaw, 0.0f);
	m_Camera->Render();
}

// Render 함수는 씬을 백 버퍼(Back Buffer)에 렌더링하고,
// 반사 및 굴절 렌더 투 텍스처(Render to Texture)를 사용하여 물을 렌더링하는 곳입니다.
bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMFLOAT3 scrollSpeeds, scales;
	XMFLOAT2 distortion1, distortion2, distortion3;
	float distortionScale, distortionBias;
	bool result;
	static float frameTime = 0.0f;

	// Increment the frame time counter.
	frameTime += 0.01f;
	if (frameTime > 1000.0f)
	{
		frameTime = 0.0f;
	}

	// Set the three scrolling speeds for the three different noise textures.
	scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);

	// Set the three scales which will be used to create the three different noise octave textures.
	scales = XMFLOAT3(1.0f, 2.0f, 3.0f);

	// Set the three different x and y distortion factors for the three different noise textures.
	distortion1 = XMFLOAT2(0.1f, 0.2f);
	distortion2 = XMFLOAT2(0.1f, 0.3f);
	distortion3 = XMFLOAT2(0.1f, 0.1f);

	// The the scale and bias of the texture coordinate sampling perturbation.
	distortionScale = 0.8f;
	distortionBias = 0.5f;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	float fireX = 0.0f, fireY = 0.0f, fireZ = 0.0f;
	float angle = atan2f(fireX - m_camPos.x, fireZ - m_camPos.z);

	worldMatrix = XMMatrixRotationY(angle) * XMMatrixTranslation(fireX, fireY, fireZ);

	// Turn on alpha blending for the fire transparency.
	m_Direct3D->EnableAlphaBlending();

	// Render the cube model using the texture shader.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_FireShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(0),
		m_Model->GetTexture(1), m_Model->GetTexture(2), frameTime, scrollSpeeds, scales, distortion1, distortion2, distortion3,
		distortionScale, distortionBias);
	if (!result)
	{
		return false;
	}

	// Turn off alpha blending.
	m_Direct3D->DisableAlphaBlending();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}