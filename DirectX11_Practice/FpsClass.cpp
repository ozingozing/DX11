///////////////////////////////////////////////////////////////////////////////
// 파일명: FpsClass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "FpsClass.h"

FpsClass::FpsClass()
{
}

FpsClass::FpsClass(const FpsClass& other)
{
}

FpsClass::~FpsClass()
{
}

// Initialize 함수는 모든 카운터를 0으로 설정하고 타이머를 시작합니다.
void FpsClass::Initialize()
{
    m_fps = 0;
    m_count = 0;

    // 현재 시간을 밀리초(ms) 단위로 가져와 시작 시간으로 기록합니다.
    m_startTime = timeGetTime();

    return;
}

// Frame 함수는 매 프레임마다 호출되어 프레임 카운트를 1씩 증가시킵니다.
// 만약 1초(1000ms)가 경과했다는 것을 감지하면, 현재까지 누적된 카운트를 m_fps 변수에 저장합니다.
// 그 후 카운트를 초기화하고 타이머를 다시 시작합니다.
void FpsClass::Frame()
{
    // 프레임 횟수 증가
    m_count++;

    // 현재 시간이 시작 시간으로부터 1000ms(1초) 이상 지났는지 확인합니다.
    if (timeGetTime() >= (m_startTime + 1000))
    {
        m_fps = m_count;     // 1초 동안 쌓인 카운트를 FPS 값으로 확정
        m_count = 0;         // 다음 측정을 위해 카운트 초기화

        m_startTime = timeGetTime(); // 시작 시간을 현재 시간으로 갱신
    }

    return;
}

// GetFps 함수는 지난 1초 동안 측정된 초당 프레임 수(FPS)를 반환합니다.
// 최신 FPS 수치를 화면에 실시간으로 표시하기 위해 지속적으로 호출되어야 합니다.
int FpsClass::GetFps()
{
    return m_fps;
}