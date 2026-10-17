////////////////////////////////////////////////////////////////////////////////
// Filename: frustumclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "FrustumClass.h"


FrustumClass::FrustumClass()
{
}


FrustumClass::FrustumClass(const FrustumClass& other)
{
}


FrustumClass::~FrustumClass()
{
}


/*
 * ConstructFrustum
 * - ApplicationClass에 의해 매 프레임 호출됩니다.
 * - 화면의 깊이(screenDepth), 뷰 행렬(viewMatrix), 투영 행렬(projectionMatrix)을 입력받아
 * 해당 프레임의 시야 프러스텀을 형성하는 6개의 평면(Plane) 방정식을 계산합니다.
 */
void FrustumClass::ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix, float screenDepth)
{
    XMMATRIX finalMatrix;
    XMFLOAT4X4 projMatrix, matrix;
    float zMinimum, r, t;


    // 1. 투영 행렬을 연산이 용이하도록 XMFLOAT4X4 구조체에 로드합니다.
    XMStoreFloat4x4(&projMatrix, projectionMatrix);

    // 2. 프러스텀 내의 최소 Z 거리를 계산하고, 화면 깊이(스크린 뎁스)를 반영하여 투영 행렬의 일부 성분을 업데이트합니다.
    zMinimum = -projMatrix._43 / projMatrix._33;
    r = screenDepth / (screenDepth - zMinimum);
    projMatrix._33 = r;
    projMatrix._43 = -r * zMinimum;

    // 3. 업데이트된 구조체 값을 다시 원래의 투영 행렬(projectionMatrix)에 반영합니다.
    projectionMatrix = XMLoadFloat4x4(&projMatrix);

    // 4. 뷰 행렬과 수정된 투영 행렬을 곱하여 '프러스텀 행렬'을 생성합니다.
    finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    // 5. 최종 결합 행렬을 행렬 성분별로 접근할 수 있게 XMFLOAT4X4 구조체에 로드합니다.
    XMStoreFloat4x4(&matrix, finalMatrix);

    // --- 6. 6개의 평면 방정식(Ax + By + Cz + D = 0)의 계수(x, y, z, w) 추출 및 정규화 ---

    // [근평면 (Near Plane) 계산]
    m_planes[0].x = matrix._13;
    m_planes[0].y = matrix._23;
    m_planes[0].z = matrix._33;
    m_planes[0].w = matrix._43;
    // 평면 정규화 (길이를 1로 만들어 방향성 확인을 정확하게 함)
    t = (float)sqrt((m_planes[0].x * m_planes[0].x) + (m_planes[0].y * m_planes[0].y) + (m_planes[0].z * m_planes[0].z));
    m_planes[0].x /= t;
    m_planes[0].y /= t;
    m_planes[0].z /= t;
    m_planes[0].w /= t;

    // [원평면 (Far Plane) 계산]
    m_planes[1].x = matrix._14 - matrix._13;
    m_planes[1].y = matrix._24 - matrix._23;
    m_planes[1].z = matrix._34 - matrix._33;
    m_planes[1].w = matrix._44 - matrix._43;
    // 평면 정규화
    t = (float)sqrt((m_planes[1].x * m_planes[1].x) + (m_planes[1].y * m_planes[1].y) + (m_planes[1].z * m_planes[1].z));
    m_planes[1].x /= t;
    m_planes[1].y /= t;
    m_planes[1].z /= t;
    m_planes[1].w /= t;

    // [좌평면 (Left Plane) 계산]
    m_planes[2].x = matrix._14 + matrix._11;
    m_planes[2].y = matrix._24 + matrix._21;
    m_planes[2].z = matrix._34 + matrix._31;
    m_planes[2].w = matrix._44 + matrix._41;
    // 평면 정규화
    t = (float)sqrt((m_planes[2].x * m_planes[2].x) + (m_planes[2].y * m_planes[2].y) + (m_planes[2].z * m_planes[2].z));
    m_planes[2].x /= t;
    m_planes[2].y /= t;
    m_planes[2].z /= t;
    m_planes[2].w /= t;

    // [우평면 (Right Plane) 계산]
    m_planes[3].x = matrix._14 - matrix._11;
    m_planes[3].y = matrix._24 - matrix._21;
    m_planes[3].z = matrix._34 - matrix._31;
    m_planes[3].w = matrix._44 - matrix._41;
    // 평면 정규화
    t = (float)sqrt((m_planes[3].x * m_planes[3].x) + (m_planes[3].y * m_planes[3].y) + (m_planes[3].z * m_planes[3].z));
    m_planes[3].x /= t;
    m_planes[3].y /= t;
    m_planes[3].z /= t;
    m_planes[3].w /= t;

    // [상평면 (Top Plane) 계산]
    m_planes[4].x = matrix._14 - matrix._12;
    m_planes[4].y = matrix._24 - matrix._22;
    m_planes[4].z = matrix._34 - matrix._32;
    m_planes[4].w = matrix._44 - matrix._42;
    // 평면 정규화
    t = (float)sqrt((m_planes[4].x * m_planes[4].x) + (m_planes[4].y * m_planes[4].y) + (m_planes[4].z * m_planes[4].z));
    m_planes[4].x /= t;
    m_planes[4].y /= t;
    m_planes[4].z /= t;
    m_planes[4].w /= t;

    // [하평면 (Bottom Plane) 계산]
    m_planes[5].x = matrix._14 + matrix._12;
    m_planes[5].y = matrix._24 + matrix._22;
    m_planes[5].z = matrix._34 + matrix._32;
    m_planes[5].w = matrix._44 + matrix._42;
    // 평면 정규화
    t = (float)sqrt((m_planes[5].x * m_planes[5].x) + (m_planes[5].y * m_planes[5].y) + (m_planes[5].z * m_planes[5].z));
    m_planes[5].x /= t;
    m_planes[5].y /= t;
    m_planes[5].z /= t;
    m_planes[5].w /= t;

    return;
}


/*
 * CheckPoint
 * - 단일 점(x, y, z)이 프러스텀 내부에 존재하는지 검사합니다.
 * - 4가지 알고리즘 중 가장 일반적인 형태이며, 상황에 따라 매우 효율적일 수 있습니다.
 * - 점이 6개 평면 모두의 내부에 있으면 true, 단 하나라도 평면 밖에 있으면 false를 반환합니다.
 */
bool FrustumClass::CheckPoint(float x, float y, float z)
{
    int i;

    // 점이 프러스텀의 6개 평면 내부에 있는지 평면 방정식을 통해 검사합니다.
    // 결과 값이 0 미만(< 0.0f)이면 평면의 바깥쪽에 있음을 의미합니다.
    for (i = 0; i < 6; i++)
    {
        if (((m_planes[i].x * x) + (m_planes[i].y * y) + (m_planes[i].z * z) + m_planes[i].w) < 0.0f)
        {
            return false;
        }
    }

    return true;
}


/*
 * CheckCube
 * - 정육면체(Cube)의 8개 꼭짓점 중 단 하나라도 프러스텀 내부에 걸쳐있는지 검사합니다.
 * - 입력값으로 중심점과 정육면체의 절반 크기(radius)를 받아 8개의 꼭짓점을 즉석에서 유도하여 연산합니다.
 * - 특정 평면에 대해 8개 꼭짓점이 모두 밖에 있다면 즉시 시야 밖에 있는 것으로 판단합니다.
 */
bool FrustumClass::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
    int i;

    // 6개의 각 평면에 대해 정육면체의 8개 꼭짓점 위치 관계를 테스트합니다.
    for (i = 0; i < 6; i++)
    {
        // 평면 방정식의 결과가 0 이상(>= 0.0f)인 꼭짓점이 하나라도 있다면, 
        // 해당 평면의 '내부 영역'에 일부라도 걸쳐있다는 뜻이므로 다음 평면 검사로 넘어갑니다(continue).

        // 꼭짓점 1: (x-r, y-r, z-r)
        if (m_planes[i].x * (xCenter - radius) +
            m_planes[i].y * (yCenter - radius) +
            m_planes[i].z * (zCenter - radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 2: (x+r, y-r, z-r)
        if (m_planes[i].x * (xCenter + radius) +
            m_planes[i].y * (yCenter - radius) +
            m_planes[i].z * (zCenter - radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 3: (x-r, y+r, z-r)
        if (m_planes[i].x * (xCenter - radius) +
            m_planes[i].y * (yCenter + radius) +
            m_planes[i].z * (zCenter - radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 4: (x+r, y+r, z-r)
        if (m_planes[i].x * (xCenter + radius) +
            m_planes[i].y * (yCenter + radius) +
            m_planes[i].z * (zCenter - radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 5: (x-r, y-r, z+r)
        if (m_planes[i].x * (xCenter - radius) +
            m_planes[i].y * (yCenter - radius) +
            m_planes[i].z * (zCenter + radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 6: (x+r, y-r, z+r)
        if (m_planes[i].x * (xCenter + radius) +
            m_planes[i].y * (yCenter - radius) +
            m_planes[i].z * (zCenter + radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 7: (x-r, y+r, z+r)
        if (m_planes[i].x * (xCenter - radius) +
            m_planes[i].y * (yCenter + radius) +
            m_planes[i].z * (zCenter + radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 8: (x+r, y+r, z+r)
        if (m_planes[i].x * (xCenter + radius) +
            m_planes[i].y * (yCenter + radius) +
            m_planes[i].z * (zCenter + radius) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 어떤 특정 평면에 대해 8개 꼭짓점이 단 하나도 안쪽에 있지 않다면(모두 < 0)
        // 그 오브젝트는 완전히 시야 밖(컬링 대상)이므로 false를 반환합니다.
        return false;
    }

    return true;
}


/*
 * CheckSphere
 * - 구체(Sphere)의 중심점과 반지름을 이용하여 구체가 프러스텀 내부에 걸쳐있는지 검사합니다.
 * - 중심점으로부터 특정 평면까지의 거리가 반지름보다 멀리 떨어져 있다면(-radius 미만)
 * 해당 구체는 시야 영역에 절대 보이지 않는 상태이므로 false를 반환합니다.
 * - **이 튜토리얼에서 구체들을 컬링하기 위해 메인으로 사용하는 함수입니다.**
 */
bool FrustumClass::CheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
    int i;

    // 구체의 반경이 프러스텀 내부에 걸쳐있는지 6개 평면을 돌며 전부 검사합니다.
    for (i = 0; i < 6; i++)
    {
        // 중심점에서 평면까지의 거리가 반지름 부호의 음수 값보다 작다면 평면 바깥에 완벽히 있는 상태입니다.
        if (((m_planes[i].x * xCenter) + (m_planes[i].y * yCenter) + (m_planes[i].z * zCenter) + m_planes[i].w) < -radius)
        {
            return false;
        }
    }

    return true;
}


/*
 * CheckRectangle
 * - CheckCube와 원리는 같으나, 정육면체가 아닌 직육면체(Box) 형태를 검사합니다.
 * - X, Y, Z축에 대한 각각의 반지름 크기(xSize, ySize, zSize)를 다르게 입력받아 8개의 꼭짓점을 만듭니다.
 */
bool FrustumClass::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
    int i;

    // 6개의 각 평면에 대해 직육면체의 8개 꼭짓점 위치 관계를 테스트합니다.
    for (i = 0; i < 6; i++)
    {
        // 임의의 평면에 대해 내부 영역(>= 0.0f)에 걸쳐있는 꼭짓점이 하나라도 발견되면 다음 평면으로 넘어갑니다.

        // 꼭짓점 1
        if (m_planes[i].x * (xCenter - xSize) +
            m_planes[i].y * (yCenter - ySize) +
            m_planes[i].z * (zCenter - zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 2
        if (m_planes[i].x * (xCenter + xSize) +
            m_planes[i].y * (yCenter - ySize) +
            m_planes[i].z * (zCenter - zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 3
        if (m_planes[i].x * (xCenter - xSize) +
            m_planes[i].y * (yCenter + ySize) +
            m_planes[i].z * (zCenter - zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 4
        if (m_planes[i].x * (xCenter - xSize) +
            m_planes[i].y * (yCenter - ySize) +
            m_planes[i].z * (zCenter + zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 5
        if (m_planes[i].x * (xCenter + xSize) +
            m_planes[i].y * (yCenter + ySize) +
            m_planes[i].z * (zCenter - zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 6
        if (m_planes[i].x * (xCenter + xSize) +
            m_planes[i].y * (yCenter - ySize) +
            m_planes[i].z * (zCenter + zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 7
        if (m_planes[i].x * (xCenter - xSize) +
            m_planes[i].y * (yCenter + ySize) +
            m_planes[i].z * (zCenter + zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 꼭짓점 8
        if (m_planes[i].x * (xCenter + xSize) +
            m_planes[i].y * (yCenter + ySize) +
            m_planes[i].z * (zCenter + zSize) + m_planes[i].w >= 0.0f)
        {
            continue;
        }

        // 특정 평면 바깥으로 8개 점이 완전히 밀려나 있다면 이 모델은 보이지 않으므로 false를 반환합니다.
        return false;
    }

    return true;
}