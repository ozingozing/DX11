////////////////////////////////////////////////////////////////////////////////
// 파일명: positionclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "PositionClass.h"

// 클래스 생성자는 시작할 때 비공개 멤버 변수들을 0으로 초기화합니다.
PositionClass::PositionClass()
{
    m_frameTime = 0.0f;
    m_rotationY = 0.0f;
    m_leftTurnSpeed = 0.0f;
    m_rightTurnSpeed = 0.0f;
}

PositionClass::PositionClass(const PositionClass& other)
{
}

PositionClass::~PositionClass()
{
}

// SetFrameTime 함수는 이 클래스의 프레임 속도를 설정하는 데 사용됩니다. 
// PositionClass는 해당 프레임 시간을 사용하여 뷰어가 얼마나 빠르게 이동하고 회전해야 하는지 계산합니다. 
// 이 함수는 뷰어 위치를 이동시키기 위해 이 클래스를 사용하기 전에 항상 각 프레임의 시작 부분에서 호출되어야 합니다.
void PositionClass::SetFrameTime(float time)
{
    m_frameTime = time;
    return;
}

// GetRotation은 뷰어의 Y축 회전 값을 반환합니다. 
// 이 함수는 이번 튜토리얼에서 필요한 유일한 도우미 함수이지만, 뷰어의 위치에 대한 더 많은 정보를 얻기 위해 확장할 수 있습니다.
void PositionClass::GetRotation(float& y)
{
    y = m_rotationY;
    return;
}

// 두 이동(회전) 함수는 모두 동일한 방식으로 작동합니다. 두 함수 모두 매 프레임마다 호출됩니다.
// 각 함수의 keydown 입력 변수는 사용자가 왼쪽 키를 누르고 있는지 또는 오른쪽 키를 누르고 있는지를 나타냅니다.
// 키를 누르고 있으면 최대 속도에 도달할 때까지 매 프레임마다 속도가 가속됩니다.
// 이렇게 하면 차량의 가속과 유사하게 카메라 속도가 빨라져 부드러운 움직임과 높은 반응성을 만들어냅니다.
// 마찬가지로 사용자가 키를 떼어 keydown 변수가 false가 되면, 속도가 0이 될 때까지 매 프레임마다 부드럽게 감속합니다.
// 프레임 속도와 관계없이 이동 속도가 일정하게 유지되도록 프레임 시간에 맞춰 속도를 계산합니다.
// 그런 다음 각 함수는 몇 가지 기본적인 수학 연산을 사용하여 카메라의 새 위치를 계산합니다.

void PositionClass::TurnLeft(bool keydown)
{
    // 키를 누르고 있으면 카메라가 왼쪽으로 회전하는 속도를 증가시킵니다. 그렇지 않으면 회전 속도를 줄입니다.
    if (keydown)
    {
        m_leftTurnSpeed += m_frameTime * 1.5f;

        if (m_leftTurnSpeed > (m_frameTime * 200.0f))
        {
            m_leftTurnSpeed = m_frameTime * 200.0f;
        }
    }
    else
    {
        m_leftTurnSpeed -= m_frameTime * 1.0f;

        if (m_leftTurnSpeed < 0.0f)
        {
            m_leftTurnSpeed = 0.0f;
        }
    }

    // 회전 속도를 사용하여 회전 값을 업데이트합니다.
    m_rotationY -= m_leftTurnSpeed;
    if (m_rotationY < 0.0f)
    {
        m_rotationY += 360.0f;
    }

    return;
}

void PositionClass::TurnRight(bool keydown)
{
    // 키를 누르고 있으면 카메라가 오른쪽으로 회전하는 속도를 증가시킵니다. 그렇지 않으면 회전 속도를 줄입니다.
    if (keydown)
    {
        m_rightTurnSpeed += m_frameTime * 1.5f;

        if (m_rightTurnSpeed > (m_frameTime * 200.0f))
        {
            m_rightTurnSpeed = m_frameTime * 200.0f;
        }
    }
    else
    {
        m_rightTurnSpeed -= m_frameTime * 1.0f;

        if (m_rightTurnSpeed < 0.0f)
        {
            m_rightTurnSpeed = 0.0f;
        }
    }

    // 회전 속도를 사용하여 회전 값을 업데이트합니다.
    m_rotationY += m_rightTurnSpeed;
    if (m_rotationY > 360.0f)
    {
        m_rotationY -= 360.0f;
    }

    return;
}