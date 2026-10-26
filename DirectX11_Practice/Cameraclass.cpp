#include "Cameraclass.h"

// 클래스 생성자는 카메라의 위치와 회전값을 씬(Scene)의 원점(0,0,0)으로 초기화합니다.
CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}

// SetPosition과 SetRotation 함수는 카메라의 위치와 회전값을 설정하는 데 사용됩니다.
void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

// GetPosition과 GetRotation 함수는 카메라의 현재 위치와 회전값을 호출한 함수에게 반환합니다.
XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

// Render 함수는 카메라의 위치와 회전 정보를 사용하여 뷰 행렬을 빌드하고 업데이트합니다.
// 먼저 up(상향), 위치, 회전 등을 위한 변수들을 설정합니다.
// 그 다음, 씬의 원점에서 카메라의 x, y, z 회전값을 바탕으로 카메라를 회전시킵니다.
// 회전이 올바르게 완료되면 카메라를 3D 공간상의 실제 위치로 이동(Translate)시킵니다.
// 이렇게 계산된 위치(position), 주시점(lookAt), 상향 벡터(up) 값을 사용하여 
// XMMatrixLookAtLH 함수를 호출하면, 현재 카메라의 회전과 이동이 반영된 뷰 행렬이 생성됩니다.
void CameraClass::Render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;


	// 상향(위쪽)을 가리키는 벡터를 설정합니다.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// 이를 XMVECTOR 구조체에 로드합니다.
	upVector = XMLoadFloat3(&up);

	// 월드 내의 카메라 위치를 설정합니다.
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	// 이를 XMVECTOR 구조체에 로드합니다.
	positionVector = XMLoadFloat3(&position);

	// 카메라가 기본적으로 바라보는 방향을 설정합니다.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// 이를 XMVECTOR 구조체에 로드합니다.
	lookAtVector = XMLoadFloat3(&lookAt);

	// 요(Yaw, Y축), 피치(Pitch, X축), 롤(Roll, Z축) 회전값을 라디안 단위로 설정합니다.
	// (0.0174532925f는 도(degree)를 라디안으로 변환하는 상수입니다.)
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// 요, 피치, 롤 값을 사용하여 회전 행렬을 생성합니다.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// 원점에서 시야가 올바르게 회전할 수 있도록 lookAt 벡터와 up 벡터를 회전 행렬에 의해 변환합니다.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// 회전된 카메라의 위치를 실제 관찰자의 위치로 이동시킵니다.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// 마지막으로 업데이트된 세 개의 벡터를 사용하여 뷰 행렬을 생성합니다.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);

	return;
}

// Render 함수를 호출하여 뷰 행렬(view matrix)을 생성한 후, 
// 이 GetViewMatrix 함수를 통해 업데이트된 뷰 행렬을 호출한 함수에 제공할 수 있습니다.
// 이 뷰 행렬은 HLSL 정점 셰이더에서 사용되는 세 가지 주요 행렬 중 하나가 됩니다.
void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}

// 새로운 RenderReflection 함수는 일반 Render 함수가 뷰 매트릭스를 빌드하는 것과
// 동일한 방식으로 반사 뷰 매트릭스를 빌드합니다.
// 가장 큰 차이점은 Y축 평면 역할을 할 객체의 높이(height)를 입력으로 받아,
// 그 높이를 사용하여 반사를 위해 position.y 변수를 반전시킨다는 점입니다.
// 또한 피치(pitch) 값도 반전시켜야 합니다. 이렇게 하면 셰이더에서 사용할 수 있는
// 반사 뷰 매트릭스가 빌드됩니다.
// 참고: 이 함수는 Y축 평면에서만 작동합니다.
void CameraClass::RenderReflection(float height, bool invertPitch = true)
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;


	// 위쪽을 가리키는 벡터를 설정합니다.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// XMVECTOR 구조체로 로드합니다.
	upVector = XMLoadFloat3(&up);

	// 월드에서 카메라의 위치를 설정합니다.
	position.x = m_positionX;
	position.y = -m_positionY + (height * 2.0f);
	position.z = m_positionZ;

	// XMVECTOR 구조체로 로드합니다.
	positionVector = XMLoadFloat3(&position);

	// 카메라가 기본적으로 바라보는 곳을 설정합니다.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// XMVECTOR 구조체로 로드합니다.
	lookAtVector = XMLoadFloat3(&lookAt);

	// 요(Y축), 피치(X축), 롤(Z축) 회전 값을 라디안 단위로 설정합니다.
	//pitch = (-1.0f * m_rotationX) * 0.0174532925f;  // 반사를 위해 피치 값을 반전합니다.
	pitch = (invertPitch ? -m_rotationX : m_rotationX) * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// 요, 피치, 롤 값을 사용하여 회전 매트릭스를 생성합니다.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// 뷰가 원점에서 올바르게 회전할 수 있도록 lookAt 및 up 벡터를 회전 매트릭스로 변환합니다.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// 회전된 카메라 위치를 뷰어의 위치로 평행이동시킵니다.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// 마지막으로 업데이트된 세 개의 벡터로부터 뷰 매트릭스를 생성합니다.
	m_reflectionViewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);

	return;
}

// GetReflectionViewMatrix 함수는 호출하는 함수들에게 새로운 반사 뷰 매트릭스를 반환합니다.
void CameraClass::GetReflectionViewMatrix(XMMATRIX& reflectionViewMatrix)
{
	reflectionViewMatrix = m_reflectionViewMatrix;
	return;
}