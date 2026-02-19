////////////////////////////////////////////////////////////////////////////////
// GLOBALS //
////////////////////////////////////////////////////////////////////////////////
// 글로벌 변수 (C++의 Constant Buffer와 매칭)
// [cbuffer: 상수 버퍼]
// 셰이더 외부(C++ 애플리케이션)에서 값을 전달받아 저장하는 공간입니다.
// 단일 변수(float, int 등)라도 cbuffer로 묶어서 관리하는 것이 GPU 성능과 메모리 배치 측면에서 유리합니다.
// 여기서는 매 프레임마다 동시에 업데이트되는 세 개의 행렬을 하나의 논리적 단위로 묶었습니다.
cbuffer MatrixBuffer
{
    matrix worldMatrix; // 오브젝트의 로컬 좌표를 월드 좌표로 변환하는 행렬
    matrix viewMatrix; // 월드 좌표를 카메라(시점) 좌표로 변환하는 행렬
    matrix projectionMatrix; // 카메라 좌표를 2D 화면(투영) 좌표로 변환하는 행렬
};

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS //
////////////////////////////////////////////////////////////////////////////////
// 구조체 정의 (하나의 파일에서 공유)
// [VertexInputType: 정점 셰이더 입력 구조체]
// CPU(C++ 코드)에서 셰이더로 처음 들어오는 데이터의 형식을 정의합니다.
struct VertexInputType
{
    // POSITION: 이 변수가 정점의 3D 위치 좌표임을 GPU에 알립니다.
    float4 position : POSITION;
    
    // COLOR: 이 변수가 정점의 색상 값(RGBA)임을 GPU에 알립니다.
    float4 color : COLOR;
};
// [PixelInputType: 픽셀 셰이더 입력 구조체]
// 정점 셰이더를 거쳐 나온 데이터가 픽셀 셰이더로 전달될 때의 형식을 정의합니다.
struct PixelInputType
{
    // SV_POSITION: System Value Position의 약자입니다.
    // 정점 셰이더에서 변환된 최종 화면 좌표임을 시스템(GPU)에 전달합니다.
    // 픽셀 셰이더에서는 반드시 SV_POSITION 시맨틱을 사용해야 화면에 올바르게 출력됩니다.
    float4 position : SV_POSITION;

    // COLOR: 픽셀의 색상 정보를 전달합니다.
    float4 color : COLOR;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
// 이 함수는 정점 버퍼(Vertex Buffer)에 있는 모든 정점 하나하나마다 실행됩니다.
// 입력(input): VertexInputType (정점의 위치와 색상)
// 출력(return): PixelInputType (변환된 위치와 색상을 픽셀 셰이더로 전달)
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 1. 위치 보정 (W값 설정)
    // 3D 위치(XYZ)만 읽어왔을 경우, 행렬 연산이 가능하도록 4번째 성분(w)을 1.0으로 명시합니다.
    // 이 작업이 없으면 w값이 정의되지 않아 행렬 곱셈 결과가 오류가 날 수 있습니다.
    input.position.w = 1.0f;

    // 2. MVP(World -> View -> Projection) 변환 연산
    // [World Matrix]: 물체를 가상 세계의 특정 위치/회전/크기로 배치합니다.
    output.position = mul(input.position, worldMatrix);
    
    // [View Matrix]: 카메라의 위치와 각도에 맞춰 세계를 재정렬합니다.
    output.position = mul(output.position, viewMatrix);
    
    // [Projection Matrix]: 3D 공간을 2D 화면에 투영합니다. (원근감 등 적용)
    output.position = mul(output.position, projectionMatrix);
    
    // 3. 데이터 전달
    // 입력받은 정점의 색상을 그대로 출력 구조체에 복사합니다.
    // 이 값은 픽셀 셰이더로 넘어가며 정점들 사이의 픽셀들에 보간(Interpolation)되어 채워집니다.
    output.color = input.color;
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (픽셀 셰이더)
////////////////////////////////////////////////////////////////////////////////
// 이 함수는 렌더링될 폴리곤 내의 모든 픽셀마다 실행됩니다.
// 입력(input): PixelInputType
// 출력(return): float4 (RGBA 색상 값)
// SV_TARGET: 이 반환값이 렌더 타겟(우리가 보는 화면 버퍼)에 써야 할 최종 색상임을 알리는 시맨틱입니다.
float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{ 
    // 현재 예제에서는 매우 단순하게, 
    // 정점 셰이더에서 넘어온 색상을 그대로 화면 픽셀의 색상으로 결정합니다.
    return input.color;
}