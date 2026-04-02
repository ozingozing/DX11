///////////////////////////////////////////////////////////////////////////////
// Filename: timerclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "TimerClass.h"


TimerClass::TimerClass()
{
}


TimerClass::TimerClass(const TimerClass& other)
{
}


TimerClass::~TimerClass()
{
}

// Initialize 함수는 먼저 시스템이 고주파 타이머를 지원하는지 확인합니다.
// 시스템에서 주파수 값을 반환하면 해당 값을 사용하여 매 밀리초마다
// 몇 번의 카운터 틱이 발생할지 결정합니다.
// 그런 다음 이 값을 매 프레임마다 사용하여 프레임 시간을 계산합니다.
// Initialize 함수의 마지막 부분에서는 타이밍을 시작하기 위해
// 해당 프레임의 시작 시간을 확인합니다.
bool TimerClass::Initialize()
{
    INT64 frequency;


    // Get the cycles per second speed for this system.
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    if (frequency == 0)
    {
        return false;
    }

    // Store it in floating point.
    m_frequency = (float)frequency;

    // Get the initial start time.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

    return true;
}

// Frame 함수는 메인 프로그램의 모든 실행 루프마다 호출됩니다.
// 이를 통해 루프 간 시간 차이를 계산하고
// 해당 프레임을 실행하는 데 걸린 시간을 파악할 수 있습니다.
// 해당 프레임의 시간을 조회하고 계산한 후 m_frameTime 변수에 저장하여
// 모든 호출 객체에서 동기화에 사용할 수 있도록 합니다.
// 그런 다음 현재 시간을 다음 프레임의 시작 시간으로 저장합니다.
void TimerClass::Frame()
{
    INT64 currentTime;
    INT64 elapsedTicks;


    // Query the current time.
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

    // Calculate the difference in time since the last time we queried for the current time.
    elapsedTicks = currentTime - m_startTime;

    // Calculate the frame time.
    m_frameTime = (float)elapsedTicks / m_frequency;

    // Restart the timer.
    m_startTime = currentTime;

    return;
}

// GetTime은 계산된 가장 최근 프레임 시간을 반환합니다.
float TimerClass::GetTime()
{
    return m_frameTime;
}