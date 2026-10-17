////////////////////////////////////////////////////////////////////////////////
// 파일명: ModelListClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ModelListClass.h"

ModelListClass::ModelListClass()
{
    m_ModelInfoList = 0;
}

ModelListClass::ModelListClass(const ModelListClass& other)
{
}

ModelListClass::~ModelListClass()
{
}

void ModelListClass::Initialize(int numModels)
{
    int i;

    // 먼저 사용할 모델의 수를 저장한 다음 ModelInfoType 구조체를 사용하여 모델의 목록 배열을 생성합니다.

    // 모델의 개수를 저장합니다.
    m_modelCount = numModels;

    // 모델 정보의 목록 배열을 생성합니다.
    m_ModelInfoList = new ModelInfoType[m_modelCount];

    // 현재 시간을 사용하여 난수 생성기를 초기화하고, 모델의 위치를 무작위로 생성하여 목록 배열에 저장합니다.

    // 현재 시간을 사용하여 난수 생성기를 초기화합니다.
    srand((unsigned int)time(NULL));

    // 모든 모델을 순회하며 위치를 무작위로 생성합니다.
    for (i = 0; i < m_modelCount; i++)
    {
        // 뷰어 앞쪽에 위치하도록 모델의 무작위 위치를 생성합니다.
        m_ModelInfoList[i].positionX = (((float)rand() - (float)rand()) / RAND_MAX) * 10.0f;
        m_ModelInfoList[i].positionY = (((float)rand() - (float)rand()) / RAND_MAX) * 10.0f;
        m_ModelInfoList[i].positionZ = ((((float)rand() - (float)rand()) / RAND_MAX) * 10.0f) + 5.0f;
    }

    return;
}

// Shutdown 함수는 모델 정보 목록 배열을 해제합니다.
void ModelListClass::Shutdown()
{
    // 모델 정보 목록을 메모리에서 해제합니다.
    if (m_ModelInfoList)
    {
        delete[] m_ModelInfoList;
        m_ModelInfoList = 0;
    }

    return;
}

// GetModelCount 함수는 이 클래스가 정보를 유지 관리하고 있는 모델의 개수를 반환합니다.
int ModelListClass::GetModelCount()
{
    return m_modelCount;
}

// GetData 함수는 입력으로 주어진 인덱스 위치에 있는 모델의 위치 데이터를 추출합니다.
void ModelListClass::GetData(int index, float& positionX, float& positionY, float& positionZ)
{
    positionX = m_ModelInfoList[index].positionX;
    positionY = m_ModelInfoList[index].positionY;
    positionZ = m_ModelInfoList[index].positionZ;
    return;
}