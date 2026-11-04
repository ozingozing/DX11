////////////////////////////////////////////////////////////////////////////////
// Filename: ApplicationClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ApplicationClass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_GroundModel = 0;
	m_WallModel = 0;
	m_BathModel = 0;
	m_WaterModel = 0;
	m_Light = 0;
	m_RefractionTexture = 0;
	m_ReflectionTexture = 0;
	m_LightShader = 0;
	m_RefractionShader = 0;
	m_WaterShader = 0;
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

	/*m_Camera->SetPosition(0.f, 15.0f, 0.0f);
	m_Camera->SetRotation(90.0f, 0.0f, 0.0f);*/
	m_Camera->SetPosition(-4.0f, 5.0f, -4.0f);
	m_Camera->SetRotation(5.f, 45.0f, 0.0f);
	m_camPos = XMFLOAT3(-15.0f, 6.0f, -15.0f);
	m_yaw = 45.0f;
	m_pitch = 15.0f;

	// 1. Ground
	// 2. Wall
	// 3. Bath
	// 4. Water

	// 1. Set the file name of the model.
	strcpy_s(modelFilename, "../Resource/ground.txt");
	// Set the file name of the textures.
	strcpy_s(textureFilename1, "../Resource/ground01.tga");
	// Create and initialize the ground model object.
	m_GroundModel = new ModelClass;
	result = m_GroundModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the ground model object.", L"Error", MB_OK);
		return false;
	}

	// 2. Set the file names of the wall model.
	strcpy_s(modelFilename, "../Resource/wall.txt");
	strcpy_s(textureFilename1, "../Resource/wall01.tga");
	// Create and initialize the wall model object.
	m_WallModel = new ModelClass;
	result = m_WallModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the wall model object.", L"Error", MB_OK);
		return false;
	}

	// 3. Set the file names of the bath model.
	strcpy_s(modelFilename, "../Resource/bath.txt");
	strcpy_s(textureFilename1, "../Resource/marble01.tga");
	// Create and initialize the bath model object.
	m_BathModel = new ModelClass;
	result = m_BathModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bath model object.", L"Error", MB_OK);
		return false;
	}

	// 4. Set the file names of the water model.
	strcpy_s(modelFilename, "../Resource/water.txt");
	strcpy_s(textureFilename1, "../Resource/water01.tga");
	// Create and initialize the water model object.
	m_WaterModel = new ModelClass;
	result = m_WaterModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the water model object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the light object.
	m_Light = new LightClass;
	m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, -1.0f, 0.5f);

	// Create and initialize the refraction render to texture object.
	m_RefractionTexture = new RenderTextureClass;
	result = m_RefractionTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR, 1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the refraction render texture object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the reflection render to texture object.
	m_ReflectionTexture = new RenderTextureClass;
	result = m_ReflectionTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR, 1);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the reflection render texture object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the light shader object.
	m_LightShader = new LightShaderClass;
	result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}
	// Create and initialize the refraction shader object.
	m_RefractionShader = new RefractionShaderClass;
	result = m_RefractionShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the refraction shader object.", L"Error", MB_OK);
		return false;
	}
	// Create and initialize the water shader object.
	m_WaterShader = new WaterShaderClass;
	result = m_WaterShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the water shader object.", L"Error", MB_OK);
		return false;
	}

	// Set the height of the water.
	m_waterHeight = 2.75f;

	// Initialize the position of the water.
	m_waterTranslation = 0.0f;

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the water shader object.
	if (m_WaterShader)
	{
		m_WaterShader->Shutdown();
		delete m_WaterShader;
		m_WaterShader = 0;
	}

	// Release the refraction shader object.
	if (m_RefractionShader)
	{
		m_RefractionShader->Shutdown();
		delete m_RefractionShader;
		m_RefractionShader = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the reflection render texture object.
	if (m_ReflectionTexture)
	{
		m_ReflectionTexture->Shutdown();
		delete m_ReflectionTexture;
		m_ReflectionTexture = 0;
	}

	// Release the refraction render texture object.
	if (m_RefractionTexture)
	{
		m_RefractionTexture->Shutdown();
		delete m_RefractionTexture;
		m_RefractionTexture = 0;
	}

	// Release the light object.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// Release the water model object.
	if (m_WaterModel)
	{
		m_WaterModel->Shutdown();
		delete m_WaterModel;
		m_WaterModel = 0;
	}

	// Release the bath model object.
	if (m_BathModel)
	{
		m_BathModel->Shutdown();
		delete m_BathModel;
		m_BathModel = 0;
	}

	// Release the wall model object.
	if (m_WallModel)
	{
		m_WallModel->Shutdown();
		delete m_WallModel;
		m_WallModel = 0;
	}

	// Release the ground model object.
	if (m_GroundModel)
	{
		m_GroundModel->Shutdown();
		delete m_GroundModel;
		m_GroundModel = 0;
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
	
	UpdateCamera(Input);

	// Update the position of the water to simulate motion.
	m_waterTranslation += 0.001f;
	if (m_waterTranslation > 1.0f)
	{
		m_waterTranslation -= 1.0f;
	}

	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.25f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	// Render the refraction of the scene to a texture.
	result = RenderRefractionToTexture();
	if (!result)
	{
		return false;
	}
	// Render the reflection of the scene to a texture.
	result = RenderReflectionToTexture();
	if (!result)
	{
		return false;
	}

	// Render the final graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

// RenderRefractionToTexture 함수는 씬의 굴절(Refraction) 결과를
// 렌더 투 텍스처(Render to Texture)로 렌더링합니다.
// 씬의 굴절은 수면 아래에 있는 모든 것을 의미하며,
// 수조(Bath) 모델 아래에는 수조 모델 자체만 보이므로 선택적으로 수조 모델만 렌더링하면 됩니다.
bool ApplicationClass::RenderRefractionToTexture()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMFLOAT4 clipPlane;
	bool result;

	// 예상대로 클립 평면(Clip Plane)을 사용하여 수면 평면 위쪽에 있는 모든 것을 잘라내고(Clip)
	// 그 아래에 있는 것만 렌더링합니다.
	// 하지만 클립 평면을 물의 높이보다 살짝 위쪽으로 평행이동시킨 것을 볼 수 있습니다.
	// 그 이유는 이 기법에 가장자리 아티팩트 문제가 발생할 수 있기 때문인데,
	// 노멀 맵 샘플링 위치로 평행이동할 때 나타나는 어두운 가장자리의 시인성을 줄이기 위해
	// (클립 평면을 살짝 위로 이동함으로써) 더 많은 색상 데이터를 사용할 수 있는 범위 내에서
	// 샘플링이 이루어지도록 강제하는 것입니다.
	// 이 프로그램을 실행할 때 일반적인 물의 높이를 그대로 사용해 보고,
	// 정상적으로 발생하는 아티팩트 현상을 직접 확인해 보세요.

	// 수면 위쪽의 모든 것을 잘라내기 위해 물의 높이를 기준으로 클리핑 평면을 설정합니다.
	clipPlane = m_flipClip
		? XMFLOAT4(0.0f, -1.0f, 0.0f, m_waterHeight /*+ 0.1f*/)    // 바꾼 값: 잘리는 것 없음
		: XMFLOAT4(0.0f, -1.0f, 0.0f, m_waterHeight + 0.1f);   // 원래: 수면 위 잘림

	// 렌더 타겟을 굴절 렌더 투 텍스처로 설정하고 내용을 지웁니다.
	m_RefractionTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_RefractionTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치를 기반으로 뷰 매트릭스를 생성합니다.
	m_Camera->Render();

	// 카메라 및 Direct3D 객체로부터 월드, 뷰, 투영 매트릭스를 가져옵니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 수조(Bath) 모델이 렌더링될 위치로 이동(Translation)시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	// 굴절 셰이더를 사용하여 수조 모델을 렌더링합니다.
	m_BathModel->Render(m_Direct3D->GetDeviceContext());

	result = m_RefractionShader->Render(m_Direct3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_BathModel->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), clipPlane);
	if (!result)
	{
		return false;
	}

	// 렌더 타겟을 더 이상 렌더 투 텍스처가 아닌 원래의 백 버퍼로 재설정합니다. 그리고 뷰포트도 원래대로 되돌립니다.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

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

	// 4. 물 아래로는 못내려가게 제한(반사카메라가 뒤집히는 걸 방지)
	if (m_camPos.y < m_waterHeight + 0.2f) m_camPos.y = m_waterHeight + 0.2f;

	// 5. 카메라에 적용
	m_Camera->SetPosition(m_camPos.x, m_camPos.y, m_camPos.z);
	m_Camera->SetRotation(m_pitch, m_yaw, 0.0f);
}

// RenderReflectionToTexture 함수는
// 씬의 반사(Reflection) 결과를 렌더 투 텍스처(Render to Texture)로 렌더링합니다.
// 씬의 반사는 수면 위에 있는 모든 것을 의미하며,
// 물에 반사될 수 있는 것은 벽(Wall) 모델뿐이므로 벽 모델만 선택적으로 렌더링하면 됩니다.
bool ApplicationClass::RenderReflectionToTexture()
{
	XMMATRIX worldMatrix, reflectionViewMatrix, projectionMatrix;
	bool result;


	// 렌더 타겟을 반사 렌더 투 텍스처로 설정하고 내용을 지웁니다.
	m_ReflectionTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());
	m_ReflectionTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 반사 행렬(Reflection Matrix)은 물의 높이를 사용하여 설정됩니다.

	// 카메라를 사용하여 반사를 렌더링하고 반사 뷰 매트릭스를 생성합니다.
	m_Camera->RenderReflection(m_waterHeight, true);

	// 일반 뷰 매트릭스 대신 카메라 반사 뷰 매트릭스를 가져옵니다.
	m_Camera->GetReflectionViewMatrix(reflectionViewMatrix);

	// Direct3D 객체로부터 월드 및 투영 매트릭스를 가져옵니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 벽(Wall) 모델이 렌더링될 위치로 이동(Translation)시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	// 라이트 셰이더와 반사 뷰 매트릭스를 사용하여 벽 모델을 렌더링합니다.
	m_WallModel->Render(m_Direct3D->GetDeviceContext());

	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, reflectionViewMatrix, projectionMatrix, m_WallModel->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	// 렌더 타겟을 더 이상 렌더 투 텍스처가 아닌 원래의 백 버퍼로 재설정합니다. 그리고 뷰포트도 원래대로 되돌립니다.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

	return true;
}

// Render 함수는 씬을 백 버퍼(Back Buffer)에 렌더링하고,
// 반사 및 굴절 렌더 투 텍스처(Render to Texture)를 사용하여 물을 렌더링하는 곳입니다.
bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;
	bool result;


	// 씬을 시작하기 위해 버퍼를 지웁니다.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치를 기반으로 뷰 매트릭스를 생성합니다.
	m_Camera->Render();

	// 카메라 및 Direct3D 객체로부터 월드, 뷰, 투영 매트릭스를 가져옵니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 먼저 지면(Ground) 모델을 렌더링합니다.

	// 지면 모델이 렌더링될 위치로 이동(Translation)시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	// 지면 모델 버텍스 및 인덱스 버퍼를 그래픽 파이프라인에 올려 그리기 준비를 합니다.
	m_GroundModel->Render(m_Direct3D->GetDeviceContext());

	// 라이트 셰이더를 사용하여 지면 모델을 렌더링합니다.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_GroundModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_GroundModel->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	// 월드 매트릭스를 리셋합니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	// 다음으로 벽(Wall) 모델을 렌더링합니다.

	// 벽 모델이 렌더링될 위치로 이동시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

	// 벽 모델 버텍스 및 인덱스 버퍼를 그래픽 파이프라인에 올려 그리기 준비를 합니다.
	m_WallModel->Render(m_Direct3D->GetDeviceContext());

	// 라이트 셰이더를 사용하여 벽 모델을 렌더링합니다.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_WallModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_WallModel->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	// 월드 매트릭스를 리셋합니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	// 수조(Bath) 모델을 렌더링합니다.

	// 수조 모델이 렌더링될 위치로 이동시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	// 수조 모델 버텍스 및 인덱스 버퍼를 그래픽 파이프라인에 올려 그리기 준비를 합니다.
	m_BathModel->Render(m_Direct3D->GetDeviceContext());

	// 라이트 셰이더를 사용하여 수조 모델을 렌더링합니다.
	result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_BathModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_BathModel->GetTexture(),
		m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor());
	if (!result)
	{
		return false;
	}

	// 월드 매트릭스를 리셋합니다.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	// 마지막으로 반사 행렬, 렌더 투 텍스처, 이동(Translation) 값, 그리고 반사/굴절 스케일 값(0.01f)을 사용하여 물(Water) 모델을 렌더링합니다.

	// 카메라 반사 뷰 매트릭스를 가져옵니다.
	m_Camera->GetReflectionViewMatrix(reflectionMatrix);

	// 물 모델이 렌더링될 위치로 이동시킵니다.
	worldMatrix = XMMatrixTranslation(0.0f, m_waterHeight, 0.0f);

	// 물 모델 버텍스 및 인덱스 버퍼를 그래픽 파이프라인에 올려 그리기 준비를 합니다.
	m_WaterModel->Render(m_Direct3D->GetDeviceContext());

	// 물 셰이더를 사용하여 물 모델을 렌더링합니다.
	result = m_WaterShader->Render(m_Direct3D->GetDeviceContext(), m_WaterModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix,
		m_ReflectionTexture->GetShaderResourceView(), m_RefractionTexture->GetShaderResourceView(), m_WaterModel->GetTexture(),
		m_waterTranslation, 0.01f, m_Camera->GetPosition());
	if (!result)
	{
		return false;
	}

	// 렌더링된 씬을 화면에 출력(Present)합니다.
	m_Direct3D->EndScene();

	return true;
}