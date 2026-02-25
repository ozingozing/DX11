/////////////
// GLOBALS //
/////////////

// 행렬 버퍼: 정점의 위치를 3D 공간으로 변환하기 위한 행렬들
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // 월드 변환 행렬
    matrix viewMatrix; // 뷰 변환 행렬
    matrix projectionMatrix; // 투영 변환 행렬
};

// 조명 버퍼: 광원의 색상과 방향 정보 (LightClass에서 설정됨)
cbuffer LightBuffer : register(b1)
{
    float4 diffuseColor; // 확산광 색상
    float3 lightDirection; // 광원이 비추는 방향
    float padding; // 16바이트 정렬을 위한 패딩
    float4 ambientColor;
};

//////////////
// TYPEDEFS //
//////////////

// 정점 셰이더 입력 구조체
struct VertexInputType
{
    float4 position : POSITION; // 정점 위치
    float2 tex : TEXCOORD0; // 텍스처 좌표
    float3 normal : NORMAL; // 법선 벡터 (조명 계산용)
};

// 픽셀 셰이더 입력 구조체 (정점 셰이더의 출력)
struct PixelInputType
{
    float4 position : SV_POSITION; // 화면상의 투영된 위치
    float2 tex : TEXCOORD0; // 보간된 텍스처 좌표
    float3 normal : NORMAL; // 월드 공간에서의 법선 벡터
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 행렬 연산을 위해 w 성분을 1.0으로 설정
    input.position.w = 1.0f;

    // 정점의 위치를 월드 -> 뷰 -> 투영 공간 순으로 변환
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 텍스처 좌표를 픽셀 셰이더로 전달
    output.tex = input.tex;

    // 법선 벡터를 월드 공간으로 변환 (회전/스케일 반영)
    // 위치 이동을 배제하기 위해 float3x3으로 캐스팅하여 연산함
    output.normal = mul(input.normal, (float3x3) worldMatrix);

    // 변환된 법선 벡터를 정규화(단위 벡터화)
    output.normal = normalize(output.normal);

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (픽셀 셰이더)
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture : register(t0); // 텍스처 리소스
SamplerState SampleType : register(s0); // 샘플러 상태

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // 1. 샘플러를 사용하여 해당 좌표의 텍스처 색상을 추출
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // 2. 조명 계산을 위해 광원 방향을 반전 (광원에서 물체가 아닌, 물체에서 광원을 향하도록)
    lightDir = -lightDirection;

    // 3. 법선 벡터와 광원 방향 벡터의 내적(Dot Product)을 통해 빛의 세기 계산
    // 두 벡터가 일치할수록 1(최대 밝기), 직교하거나 반대면 0 이하가 됨
    // saturate는 값을 0.0 ~ 1.0 사이로 제한함
    lightIntensity = saturate(dot(input.normal, lightDir));

    // 4. 광원의 확산색상과 계산된 세기를 곱하여 최종 조명 색상 결정
    //color = saturate(diffuseColor * lightIntensity);

    float4 diffuse = diffuseColor * lightIntensity;
    float4 ambient = ambientColor;
    
    // 5. 텍스처 색상과 조명 색상을 결합(곱셈)하여 최종 픽셀 색상 출력
    color = (ambient + diffuse) * textureColor;

    return color;
}