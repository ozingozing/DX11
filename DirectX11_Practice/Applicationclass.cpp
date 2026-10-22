////////////////////////////////////////////////////////////////////////////////
// Filename: ApplicationClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ApplicationClass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model1 = 0;
	m_Model2 = 0;
	m_TextureShader = 0;
	m_TransparentShader = 0;
	m_model2X = 1.0f;
	m_model2Y = 0.0f;
	m_model2Z = -1.0f;
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
	char modelFilename[128], textureFilename1[128], textureFilename2[128], renderString[32];
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
	strcpy_s(modelFilename, "../Resource/square.txt");
	// Set the file name of the textures.
	strcpy_s(textureFilename1, "../Resource/stone01.tga");
	strcpy_s(textureFilename2, "../Resource/dirt01.tga");


	// Create and initialize the model object.
	m_Model1 = new ModelClass;
	m_Model2 = new ModelClass;
	result = m_Model1->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		return false;
	}
	result = m_Model2->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename2);
	if (!result)
	{
		return false;
	}

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the translate shader object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the transparent shader object.
	m_TransparentShader = new TransparentShaderClass;

	result = m_TransparentShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the transparent shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the transparent shader object.
	if (m_TransparentShader)
	{
		m_TransparentShader->Shutdown();
		delete m_TransparentShader;
		m_TransparentShader = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the model object.
	if (m_Model2)
	{
		m_Model2->Shutdown();
		delete m_Model2;
		m_Model2 = 0;
	}

	// Release the model object.
	if (m_Model1)
	{
		m_Model1->Shutdown();
		delete m_Model1;
		m_Model1 = 0;
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

	if (Input->IsKeyDown(DIK_LEFT))  m_model2X -= moveSpeed;
	if (Input->IsKeyDown(DIK_RIGHT)) m_model2X += moveSpeed;
	if (Input->IsKeyDown(DIK_W))     m_model2Z += moveSpeed;   // +z = 카메라에서 멀어짐
	if (Input->IsKeyDown(DIK_S))     m_model2Z -= moveSpeed;   // -z = 카메라 쪽으로

	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}

	m_Camera->Render();

	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.25f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	// Render the final graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	float blendAmount;
	bool result;

	// Set the blending amount to 50%.
	blendAmount = 0.5f;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// Render the first model that is using the dirt texture using the regular texture shader.
	m_Model1->Render(m_Direct3D->GetDeviceContext());

	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model1->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model1->GetTexture());
	if (!result)
	{
		return false;
	}

	// Translate to the right by one unit and towards the camera by one unit.
	worldMatrix = XMMatrixTranslation(m_model2X, m_model2Y, m_model2Z);

	// Turn on alpha blending for the transparency to work.
	m_Direct3D->EnableAlphaBlending();

	// Render the second square model with the stone texture and use the 50% blending amount for transparency.
	m_Model2->Render(m_Direct3D->GetDeviceContext());

	result = m_TransparentShader->Render(m_Direct3D->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model2->GetTexture(), blendAmount);
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

//bool ApplicationClass::Render(float rotation)
//{
//	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
//	XMFLOAT4 clipPlane;
//	bool result;
//
//	clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
//
//	// Clear the buffers to begin the scene.
//	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
//
//	// Get the world, view, and projection matrices from the camera and d3d objects.
//	m_Direct3D->GetWorldMatrix(worldMatrix);
//	m_Camera->GetViewMatrix(viewMatrix);
//	m_Direct3D->GetProjectionMatrix(projectionMatrix);
//
//	// Rotate the world matrix by the rotation value so that the cube will spin.
//	worldMatrix = XMMatrixRotationY(rotation);
//
//	// Render the model using the texture shader.
//	m_Model->Render(m_Direct3D->GetDeviceContext());
//
//	/*result = m_ClipPlaneShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
//		m_Model->GetTexture(), clipPlane);
//	if (!result)
//	{
//		return false;
//	}*/
//
//	// Present the rendered scene to the screen.
//	m_Direct3D->EndScene();
//
//	return true;
//}

bool ApplicationClass::Render(float textureTranslation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	//Now render the model with the TranslateShaderClass and send in the texture translation value.

	// Render the model using the translate shader.
	/*m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_TranslateShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(), textureTranslation);
	if (!result)
	{
		return false;
	}*/

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}