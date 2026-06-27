/////////////
// DEFINES //
/////////////

/////////////
// GLOBALS //
/////////////

// 행렬 버퍼: 정점을 3D 공간으로 변환하기 위한 행렬들
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // 월드 변환 행렬
    matrix viewMatrix; // 뷰 변환 행렬
    matrix projectionMatrix; // 투영 변환 행렬
};

// HLSL은 정의된 요소 수를 사용하여 배열을 사용할 수 있습니다.
// 이 버퍼는 4개 점광원(Point Light)의 위치를 저장합니다.
cbuffer LightPositionBuffer : register(b1)
{
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
};

//////////////
// TYPEDEFS //
//////////////

// 정점 셰이더 입력 구조체
struct VertexInputType
{
    float4 position : POSITION; // 정점 위치
    float2 tex : TEXCOORD0; // 텍스처 좌표
    float3 normal : NORMAL; // 조명 계산을 위한 법선(Normal) 벡터
};

// 픽셀 셰이더 입력 구조체 (정점 셰이더의 출력)
struct PixelInputType
{
    float4 position : SV_POSITION; // 투영된 화면상의 위치
    float2 tex : TEXCOORD0; // 보간된 텍스처 좌표
    float3 normal : NORMAL; // 월드 공간에서의 법선 벡터
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    int i;

    // 행렬 곱셈을 위해 w 성분을 1.0으로 설정
    input.position.w = 1.0f;

    // 위치를 월드 -> 뷰 -> 투영 공간 순으로 변환
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 텍스처 좌표를 픽셀 셰이더로 전달
    output.tex = input.tex;

    output.normal = mul(input.normal, (float3x3) worldMatrix);
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

    // 현재 UV 좌표에서 텍스처 색상을 샘플링함
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    input.normal = normalize(input.normal);

    lightDir = -lightDirection;
    lightDir = normalize(lightDir);

    lightIntensity = saturate(dot(input.normal, lightDir));

    // 모든 광원의 기여도를 합산함
    color = diffuseColor * lightIntensity;
    color = saturate(color) * textureColor;

    return color;
}