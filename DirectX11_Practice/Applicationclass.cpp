////////////////////////////////////////////////////////////////////////////////
// Filename: ApplicationClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ApplicationClass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_CubeModel = 0;
	m_FloorModel = 0;
	m_RenderTexture = 0;
	m_TextureShader = 0;
	m_ReflectionShader = 0;
	m_ClipPlaneShader = 0;

	m_invertPitch = true;
	m_prevP = false;
	m_useClip = false;
	m_prevC = false;
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
	char modelFilename[128], textureFilename1[128];
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

	m_Camera->SetPosition(0.0f, 3.0f, -10.0f);   // y를 0 → 3으로 올림
	m_Camera->SetRotation(15.0f, 0.0f, 0.0f);    // x축 15° = 아래로 내려다봄
	m_Camera->Render();

	// Set the file name of the model.
	strcpy_s(modelFilename, "../Resource/cube.txt");
	// Set the file name of the textures.
	strcpy_s(textureFilename1, "../Resource/stone01.tga");


	// Create and initialize the cube model object.
	m_CubeModel = new ModelClass;
	result = m_CubeModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error", MB_OK);
		return false;
	}

	// Set the file name of the floor model.
	strcpy_s(modelFilename, "../Resource/floor.txt");
	// Set the file name of the texture.
	strcpy_s(textureFilename1, "../Resource/blue01.tga");

	// Create and initialize the floor model object.
	m_FloorModel = new ModelClass;
	result = m_FloorModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the floor model object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the render to texture object.
	m_RenderTexture = new RenderTextureClass;
	result = m_RenderTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR, 1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the render texture object.", L"Error", MB_OK);
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

	// Create and initialize the reflection shader object.
	m_ReflectionShader = new ReflectionShaderClass;
	result = m_ReflectionShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the reflection shader object.", L"Error", MB_OK);
		return false;
	}
	m_ClipPlaneShader = new ClipPlaneShaderClass;
	if (!m_ClipPlaneShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		return false;
	}



	XMMATRIX proj;
	m_Direct3D->GetProjectionMatrix(proj);
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, proj);

	char buf[128];
	sprintf_s(buf, "yScale = %f, xScale = %f\n", m._22, m._11);
	OutputDebugStringA(buf);

	return true;
}


void ApplicationClass::Shutdown()
{
	if(m_ClipPlaneShader)
	{
		m_ClipPlaneShader->Shutdown();
		delete m_ClipPlaneShader;
		m_ClipPlaneShader = 0;
	}

	// Release the reflection shader object.
	if (m_ReflectionShader)
	{
		m_ReflectionShader->Shutdown();
		delete m_ReflectionShader;
		m_ReflectionShader = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the render texture object.
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	// Release the floor model object.
	if (m_FloorModel)
	{
		m_FloorModel->Shutdown();
		delete m_FloorModel;
		m_FloorModel = 0;
	}

	// Release the cube model object.
	if (m_CubeModel)
	{
		m_CubeModel->Shutdown();
		delete m_CubeModel;
		m_CubeModel = 0;
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

	//if (Input->IsKeyDown(DIK_LEFT))  m_model2X -= moveSpeed;
	//if (Input->IsKeyDown(DIK_RIGHT)) m_model2X += moveSpeed;
	//if (Input->IsKeyDown(DIK_W))     m_model2Z += moveSpeed;   // +z = 카메라에서 멀어짐
	//if (Input->IsKeyDown(DIK_S))     m_model2Z -= moveSpeed;   // -z = 카메라 쪽으로

	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}
	bool nowP = Input->IsKeyDown(DIK_P);
	if (nowP && !m_prevP)
		m_invertPitch = !m_invertPitch;   // 누르는 순간 한 번만 토글
	m_prevP = nowP;

	bool nowC = Input->IsKeyDown(DIK_C);
	if (nowC && !m_prevC)
		m_useClip = !m_useClip;
	m_prevC = nowC;


	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.25f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	// Render the entire scene as a reflection to the texture first.
	result = RenderReflectionToTexture(rotation);
	if (!result)
	{
		return false;
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
	//XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	//float blendAmount;
	//bool result;

	//// Set the blending amount to 50%.
	//blendAmount = 0.5f;

	//// Clear the buffers to begin the scene.
	//m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//// Get the world, view, and projection matrices from the camera and d3d objects.
	//m_Direct3D->GetWorldMatrix(worldMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	//// Render the first model that is using the dirt texture using the regular texture shader.
	//m_Model1->Render(m_Direct3D->GetDeviceContext());

	//result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model1->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model1->GetTexture());
	//if (!result)
	//{
	//	return false;
	//}

	//// Translate to the right by one unit and towards the camera by one unit.
	//worldMatrix = XMMatrixTranslation(m_model2X, m_model2Y, m_model2Z);

	//// Turn on alpha blending for the transparency to work.
	//m_Direct3D->EnableAlphaBlending();

	//// Render the second square model with the stone texture and use the 50% blending amount for transparency.
	//m_Model2->Render(m_Direct3D->GetDeviceContext());

	//result = m_TransparentShader->Render(m_Direct3D->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model2->GetTexture(), blendAmount);
	//if (!result)
	//{
	//	return false;
	//}

	//// Turn off alpha blending.
	//m_Direct3D->DisableAlphaBlending();

	//// Present the rendered scene to the screen.
	//m_Direct3D->EndScene();

	return true;
}

bool ApplicationClass::RenderReflectionToTexture(float rotation)
{
	XMFLOAT4 clipPlane = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.5f);   // f = y + 1.5 → 바닥 위만 남김
	XMMATRIX worldMatrix, reflectionViewMatrix, projectionMatrix;
	bool result;


	// 렌더 타겟을 렌더 투 텍스처(Render-to-Texture)로 설정하고 내용을 지웁니다.
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 씬을 텍스처로 렌더링하기 전에, 바닥의 위치(-1.5)를 높이 입력 변수로 사용하여 반사 행렬(reflection matrix)을 생성해야 합니다.

	// 카메라를 사용하여 반사 행렬을 계산합니다.
	m_Camera->RenderReflection(-1.5f, m_invertPitch);

	// 평소처럼 씬을 렌더링하되, 일반 뷰 매트릭스 대신 반사 행렬을 사용합니다. 
	// 또한 반사될 대상(회전하는 큐브)만 렌더링하면 되므로 바닥은 렌더링할 필요가 없습니다.

	// 일반 뷰 매트릭스 대신 카메라 반사 뷰 매트릭스를 가져옵니다.
	m_Camera->GetReflectionViewMatrix(reflectionViewMatrix);

	// 월드 및 투영 매트릭스를 가져옵니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 큐브가 회전하도록 회전 값에 따라 월드 매트릭스를 회전시킵니다.
	//worldMatrix *= XMMatrixTranslation(0.0f, -1.5f, 0.0f);
	worldMatrix = XMMatrixRotationY(rotation) * XMMatrixTranslation(0.0f, 0.5f, 0.0f);

	// 텍스처 셰이더와 반사 뷰 매트릭스를 사용하여 큐브 모델을 렌더링합니다.
	m_CubeModel->Render(m_Direct3D->GetDeviceContext());

	if (m_useClip)
		result = m_ClipPlaneShader->Render(m_Direct3D->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix,
			reflectionViewMatrix, projectionMatrix,
			m_CubeModel->GetTexture(), clipPlane);
	else
		result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix,
			reflectionViewMatrix, projectionMatrix,
			m_CubeModel->GetTexture());
	if (!result) return false;

	// 렌더 타겟을 더 이상 렌더 투 텍스처가 아닌 원래의 백 버퍼로 재설정합니다. 그리고 뷰포트도 원래대로 되돌립니다.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

	return true;
}

bool ApplicationClass::Render(float rotation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionViewMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// Rotate the world matrix by the rotation value so that the cube will spin.
	//worldMatrix = XMMatrixRotationY(rotation);
	worldMatrix = XMMatrixRotationY(rotation) * XMMatrixTranslation(0.0f, 0.5f, 0.0f);

	// Render the cube model using the texture shader and the regular view matrix.
	m_CubeModel->Render(m_Direct3D->GetDeviceContext());

	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_CubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_CubeModel->GetTexture());
	if (!result)
	{
		return false;
	}
	
	// Now get the world matrix again and translate down for the floor model to render underneath the cube.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranslation(0.0f, -1.5f, 0.0f);

	// Get the camera reflection view matrix for the reflection shader.
	m_Camera->GetReflectionViewMatrix(reflectionViewMatrix);

	// Render the floor model using the reflection shader, reflection render texture, and reflection view matrix.
	m_FloorModel->Render(m_Direct3D->GetDeviceContext());

	result = m_ReflectionShader->Render(m_Direct3D->GetDeviceContext(), m_FloorModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_FloorModel->GetTexture(), m_RenderTexture->GetShaderResourceView(), reflectionViewMatrix);
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}