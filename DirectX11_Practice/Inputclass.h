////////////////////////////////////////////////////////////////////////////////
// 파일명: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

// Direct Input 버전을 정의합니다. 
// 정의하지 않으면 컴파일러가 기본값인 버전 8로 설정한다는 경고 메시지를 출력합니다.
#define DIRECTINPUT_VERSION 0x0800

///////////////////////////////
// 라이브러리 링크 (LINKING) //
///////////////////////////////
// Direct Input 작동에 필요한 두 개의 라이브러리입니다.
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//////////////
// 헤더 포함 //
//////////////
#include <dinput.h>

////////////////////////////////////////////////////////////////////////////////
// 클래스명: InputClass
////////////////////////////////////////////////////////////////////////////////
class InputClass
{
public:
    InputClass();
    InputClass(const InputClass&);
    ~InputClass();

    // 입력 장치 초기화, 종료, 프레임 업데이트 함수
    bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
    void Shutdown();
    bool Frame();

    // 상태 확인용 헬퍼 함수들
    bool IsEscapePressed();
    void GetMouseLocation(int& mouseX, int& mouseY);
    bool IsMousePressed();

private:
    // 실제 장치로부터 데이터를 읽어오는 내부 함수
    bool ReadKeyboard();
    bool ReadMouse();

    // 읽어온 데이터를 처리(계산)하는 함수
    void ProcessInput();

private:
    // Direct Input 메인 인터페이스와 키보드/마우스 장치 인터페이스입니다.
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;

    // 키보드와 마우스의 현재 상태(버튼 눌림 등)를 기록하는 배열과 구조체입니다.
    unsigned char m_keyboardState[256];
    DIMOUSESTATE m_mouseState;

    // 화면 크기와 마우스의 현재 좌표값을 저장합니다.
    int m_screenWidth, m_screenHeight, m_mouseX, m_mouseY;
};

#endif