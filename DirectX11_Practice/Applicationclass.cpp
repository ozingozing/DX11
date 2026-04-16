////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_FontShader = 0;
	m_Font = 0;
	m_Fps = 0;
	m_FpsString = 0;
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
	char fpsString[32];
	bool result;


	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the camera object.
	m_Camera = new CameraClass;

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	// Create and initialize the font shader object.
	m_FontShader = new FontShaderClass;

	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the font object.
	m_Font = new FontClass;

	result = m_Font->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), 0);
	if (!result)
	{
		return false;
	}

	// 여기서 FpsClass를 생성하고 초기화합니다. 
	// 프로그램이 실행된 후 1초가 지나기 전까지는 정확한 FPS 값을 알 수 없으므로, 초기 FPS 값을 미리 설정해 둡니다.
	// 또한 화면에 FPS 수치를 문자열로 렌더링하기 위해 TextClass를 생성합니다.
	// 참고: FpsClass 내부에 렌더링 기능을 포함시킬 수도 있었지만, 가능한 한 코드 간의 결합도를 낮추는(Decouple) 것이 더 좋은 프로그래밍 관례입니다.

	// FPS 객체를 생성하고 초기화합니다.
	m_Fps = new FpsClass();
	m_Fps->Initialize();

	// 초기 FPS 값과 FPS 표시용 문자열을 설정합니다.
	m_previousFps = -1;
	strcpy_s(fpsString, "Fps: 0");

	// FPS 문자열을 렌더링하기 위한 텍스트 객체를 생성하고 초기화합니다.
	m_FpsString = new TextClass;

	// 텍스트 객체 초기화: (10, 10) 위치에 초록색(0.0, 1.0, 0.0)으로 텍스트를 배치합니다.
	result = m_FpsString->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, 32, m_Font, fpsString, 10, 10, 0.0f, 1.0f, 0.0f);
	if (!result)
	{
		return false;
	}
}


void ApplicationClass::Shutdown()
{
	// Release the text object for the fps string.
	if (m_FpsString)
	{
		m_FpsString->Shutdown();
		delete m_FpsString;
		m_FpsString = 0;
	}

	// Release the fps object.
	if (m_Fps)
	{
		delete m_Fps;
		m_Fps = 0;
	}

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
	// We use a new function to do the FPS updates and processing each frame.
	// Update the frames per second each frame.
	result = UpdateFps();
	if (!result)
	{
		return false;
	}

	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	// Disable the Z buffer and enable alpha blending for 2D rendering.
	m_Direct3D->TurnZBufferOff();
	m_Direct3D->EnableAlphaBlending();
	// Each frame we Render the FPS string to the screen.

	// Render the fps text string using the font shader.
	m_FpsString->Render(m_Direct3D->GetDeviceContext());

	result = m_FontShader->Render(m_Direct3D->GetDeviceContext(), m_FpsString->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
		m_Font->GetTexture(), m_FpsString->GetPixelColor());
	if (!result)
	{
		return false;
	}

	// Enable the Z buffer and disable alpha blending now that 2D rendering is complete.
	m_Direct3D->TurnZBufferOn();
	m_Direct3D->DisableAlphaBlending();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

// 새로운 UpdateFps 함수는 매 프레임마다 FPS 카운터를 업데이트합니다.
// 만약 FPS 수치가 이전 프레임과 달라졌다면, 화면에 렌더링되는 FPS 텍스트 문자열도 함께 갱신합니다.
// 또한 성능 상태에 따라 색상을 설정합니다: 60 FPS 이상은 초록색, 60 미만은 노란색, 30 미만은 빨간색입니다.
// 일반적으로 이런 함수는 사용자 인터페이스(UI) 전용 클래스에 위치해야 하지만, 
// 튜토리얼의 단순화를 위해 현재는 ApplicationClass에 추가해 두었습니다.

bool ApplicationClass::UpdateFps()
{
	int fps;
	char tempString[16], finalString[16];
	float red, green, blue;
	bool result;

	// 매 프레임마다 FPS를 업데이트합니다 (내부 카운트 증가 및 1초 체크).
	m_Fps->Frame();

	// 현재 측정된 FPS 값을 가져옵니다.
	fps = m_Fps->GetFps();

	// 이전 프레임의 FPS와 동일하다면, 텍스트 문자열을 갱신할 필요가 없으므로 그대로 리턴합니다.
	if (m_previousFps == fps)
	{
		return true;
	}

	// 다음 프레임에서의 확인을 위해 현재 FPS를 저장합니다.
	m_previousFps = fps;

	// FPS 수치가 100,000을 넘지 않도록 제한합니다 (문자열 버퍼 오버플로우 방지).
	if (fps > 99999)
	{
		fps = 99999;
	}

	// 정수형 FPS 값을 문자열 형식으로 변환합니다.
	sprintf_s(tempString, "%d", fps);

	// 출력할 최종 FPS 문자열을 구성합니다. (예: "Fps: 60")
	strcpy_s(finalString, "Fps: ");
	strcat_s(finalString, tempString);

	// 1. FPS가 60 이상이면 텍스트 색상을 초록색으로 설정합니다.
	if (fps >= 60)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// 2. FPS가 60 미만이면 텍스트 색상을 노란색으로 설정합니다.
	if (fps < 60)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// 3. FPS가 30 미만이면 텍스트 색상을 빨간색으로 설정합니다.
	if (fps < 30)
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// 새로운 문자열 정보와 색상으로 문장의 정점 버퍼를 갱신합니다.
	// 여기서 이전에 배운 Map/Unmap 로직이 내부적으로 호출됩니다.
	result = m_FpsString->UpdateText(m_Direct3D->GetDeviceContext(), m_Font, finalString, 10, 10, red, green, blue);
	if (!result)
	{
		return false;
	}

	return true;
}