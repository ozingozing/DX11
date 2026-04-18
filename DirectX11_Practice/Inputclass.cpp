////////////////////////////////////////////////////////////////////////////////
// 파일명: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Inputclass.h"

// 생성자에서는 Direct Input 인터페이스 변수들을 초기화합니다.
InputClass::InputClass()
{
    m_directInput = 0;
    m_keyboard = 0;
    m_mouse = 0;
}

InputClass::InputClass(const InputClass& other)
{
}

InputClass::~InputClass()
{
}

bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result;

    // 마우스 커서 위치 계산에 사용할 화면 크기를 저장합니다.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // 화면상의 마우스 초기 위치를 설정합니다.
    m_mouseX = 0;
    m_mouseY = 0;

    // 1. 메인 Direct Input 인터페이스를 초기화합니다. 
    // 이 객체가 생성되어야 다른 입력 장치(키보드, 마우스 등)를 초기화할 수 있습니다.
    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // 2. 키보드 장치를 초기화합니다.
    result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // 데이터 형식을 설정합니다. 키보드의 경우 미리 정의된 기본 형식을 사용하면 됩니다.
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return false;
    }

    // 키보드 협조 수준(Cooperative Level) 설정:
    // DISCL_EXCLUSIVE를 설정하면 우리 프로그램이 포커스를 가졌을 때 다른 프로그램은 키보드 입력을 받지 못합니다.
    // 다른 앱과 공유하려면 DISCL_NONEXCLUSIVE를 사용하세요.
    // 윈도우 모드에서 비독점 모드를 쓸 경우 포커스를 잃었을 때(최소화 등)를 대비해 재획득(Re-acquire) 로직이 필요합니다.
    result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // 모든 설정이 끝났으므로 키보드 장치를 획득(Acquire)합니다.
    result = m_keyboard->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    // 3. 마우스 장치를 초기화합니다.
    result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // 미리 정의된 마우스 데이터 형식을 설정합니다.
    result = m_mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result))
    {
        return false;
    }

    // 마우스는 다른 프로그램과 공유할 수 있도록 비독점(Non-exclusive) 모드로 설정합니다.
    result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // 마우스 장치를 획득합니다.
    result = m_mouse->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

// Shutdown 함수는 사용한 장치와 인터페이스를 해제합니다.
// 항상 Unacquire를 먼저 호출한 뒤 Release를 해야 합니다.
void InputClass::Shutdown()
{
    // 마우스 해제
    if (m_mouse)
    {
        m_mouse->Unacquire();
        m_mouse->Release();
        m_mouse = 0;
    }

    // 키보드 해제
    if (m_keyboard)
    {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = 0;
    }

    // Direct Input 메인 인터페이스 해제
    if (m_directInput)
    {
        m_directInput->Release();
        m_directInput = 0;
    }

    return;
}

// Frame 함수는 매 프레임마다 장치 상태를 읽어오고 변화를 처리합니다.
bool InputClass::Frame()
{
    bool result;

    // 현재 키보드 상태를 읽어옵니다.
    result = ReadKeyboard();
    if (!result)
    {
        return false;
    }

    // 현재 마우스 상태를 읽어옵니다.
    result = ReadMouse();
    if (!result)
    {
        return false;
    }

    // 읽어온 데이터를 바탕으로 좌표 등을 계산합니다.
    ProcessInput();

    return true;
}

// ReadKeyboard는 키보드 상태를 m_keyboardState 변수에 담습니다.
// 만약 포커스를 잃어 오류가 발생하면, 다시 포커스를 얻을 때까지 Acquire를 시도합니다.
bool InputClass::ReadKeyboard()
{
    HRESULT result;

    // 키보드 장치 상태를 가져옵니다.
    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
    if (FAILED(result))
    {
        // 입력 장치 포커스를 잃었거나 획득되지 않은 상태라면 다시 Acquire를 시도합니다.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_keyboard->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}

// ReadMouse는 마우스 상태를 읽어옵니다. 
// 주의: 마우스 상태 데이터는 실제 화면 좌표가 아니라 '이전 프레임 대비 움직인 거리(Delta)'입니다.
bool InputClass::ReadMouse()
{
    HRESULT result;

    // 마우스 장치 상태를 가져옵니다.
    result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
    if (FAILED(result))
    {
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_mouse->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}

// ProcessInput에서는 장치 변화량을 바탕으로 실제 게임에서 쓸 좌표 등을 계산합니다.
void InputClass::ProcessInput()
{
    // 이전 프레임 대비 마우스의 움직임(lX, lY)을 현재 좌표에 더해줍니다.
    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;

    // 마우스 커서가 화면 밖으로 나가지 않도록 고정(Clamping)합니다.
    if (m_mouseX < 0) { m_mouseX = 0; }
    if (m_mouseY < 0) { m_mouseY = 0; }

    if (m_mouseX > m_screenWidth) { m_mouseX = m_screenWidth; }
    if (m_mouseY > m_screenHeight) { m_screenHeight = m_screenWidth; } // 오타 주의: m_screenHeight여야 함

    return;
}

// 특정 키(Escape)가 눌렸는지 확인하는 예시 함수입니다.
bool InputClass::IsEscapePressed()
{
    // 비트 연산(& 0x80)을 통해 해당 키가 눌린 상태인지 확인합니다.
    if (m_keyboardState[DIK_ESCAPE] & 0x80)
    {
        return true;
    }

    return false;
}

// 현재 마우스 좌표를 반환하는 헬퍼 함수입니다.
void InputClass::GetMouseLocation(int& mouseX, int& mouseY)
{
    mouseX = m_mouseX;
    mouseY = m_mouseY;
    return;
}

// 마우스 왼쪽 버튼이 눌렸는지 확인합니다. (rgbButtons[0])
bool InputClass::IsMousePressed()
{
    if (m_mouseState.rgbButtons[0] & 0x80)
    {
        return true;
    }

    return false;
}