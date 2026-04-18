////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"

SystemClass::SystemClass()
{
	m_Input = 0;
	m_Application = 0;
}

SystemClass::SystemClass(const SystemClass&)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;


	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create and initialize the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new InputClass;
	//The first change is that the Initialize function of the updated InputClass now takes in additional variables.

	result = m_Input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);
	if (!result)
	{
		return false;
	}

	// Create and initialize the application class object.  This object will handle rendering all the graphics for this application.
	m_Application = new ApplicationClass;

	result = m_Application->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}

void SystemClass::Shutdown()
{
	// 애플리케이션 클래스 객체를 해제합니다.
	if (m_Application)
	{
		m_Application->Shutdown();
		delete m_Application;
		m_Application = 0;
	}

	// 입력 객체를 해제합니다.
	if (m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// 창문을 닫으세요.
	ShutdownWindows();

	return;
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// 메시지 구조를 초기화합니다.
	ZeroMemory(&msg, sizeof(MSG));

	// 창이나 사용자로부터 종료 메시지가 나타날 때까지 반복합니다.
	done = false;
	while (!done)
	{
		// 윈도우 메시지를 처리하세요.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Windows에서 애플리케이션 종료 신호를 보내면 종료하십시오.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// 그렇지 않으면 프레임 처리를 수행합니다.
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}

	}

	return;
}

LRESULT SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

bool SystemClass::Frame()
{
	bool result;


	// Do the input frame processing.
	result = m_Input->Frame();
	if (!result)
	{
		return false;
	}

	// Do the frame processing for the application class object.
	result = m_Application->Frame(m_Input);
	if (!result)
	{
		return false;
	}
	
	return true;
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// 이 객체에 대한 외부 포인터를 가져옵니다.	
	ApplicationHandle = this;

	// 이 애플리케이션의 인스턴스를 가져옵니다.
	m_hinstance = GetModuleHandle(NULL);

	// 애플리케이션에 이름을 지정하세요.
	m_applicationName = L"Engine";

	// 윈도우 클래스를 기본 설정으로 구성합니다.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// 윈도우 클래스를 등록하세요.
	RegisterClassEx(&wc);

	// 클라이언트 데스크톱 화면의 해상도를 확인합니다.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// 전체 화면 모드인지 창 모드인지에 따라 화면 설정을 구성하십시오.
	if (FULL_SCREEN)
	{
		// 전체 화면으로 설정할 경우 화면 크기를 사용자의 데스크톱 최대 크기로 하고 32비트로 설정하십시오.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// 화면 설정을 전체 화면으로 변경하세요.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// 창의 위치를 ​​왼쪽 상단 모서리로 설정하세요.
		posX = posY = 0;
	}
	else
	{
		// 창 모드인 경우 해상도를 800x600으로 설정하십시오.
		screenWidth = 800;
		screenHeight = 600;

		// 창을 화면 중앙에 배치하세요.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// 화면 설정이 포함된 창을 만들고 해당 창의 핸들을 가져옵니다.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// 해당 창을 화면 위로 가져와서 메인 포커스로 설정하세요.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// 마우스 커서를 숨깁니다.
	ShowCursor(false);

	return;
}

void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}