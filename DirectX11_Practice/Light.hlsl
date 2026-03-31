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
    float4 lightPosition[4];
};

// 이 버퍼는 4개 점광원의 난반사(Diffuse) 색상을 저장합니다.
cbuffer LightColorBuffer : register(b2)
{
    float4 diffuseColor[4];
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
    float3 lightPos[4] : TEXCOORD1; // 각 광원으로의 방향 벡터 배열
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

    // 법선 벡터를 월드 공간으로 변환
    // 위치 이동(Translation) 값을 무시하기 위해 float3x3으로 캐스팅하여 연산
    output.normal = mul(input.normal, (float3x3) worldMatrix);

    // 변환된 법선 벡터를 정규화(단위 벡터화)
    output.normal = normalize(output.normal);

    // 정점의 월드 공간 위치를 계산
    worldPosition = mul(input.position, worldMatrix);

    // 정점에서 각 광원으로 향하는 방향 벡터를 계산하고
    // 픽셀 셰이더로 넘기기 전에 각 벡터를 정규화함
    for (i = 0; i < 4; i++)
    {
        output.lightPos[i] = lightPosition[i].xyz - worldPosition.xyz;
        output.lightPos[i] = normalize(output.lightPos[i]);
    }

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
    float lightIntensity[4];
    float4 colorArray[4];
    float4 colorSum;
    float4 color;
    int i;

    // 현재 UV 좌표에서 텍스처 색상을 샘플링함
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // 각 점광원에 대해 빛의 강도와 난반사 기여도를 계산함
    for (i = 0; i < 4; i++)
    {
        // 법선 벡터와 광원 방향 벡터의 내적을 통해 빛의 세기 결정 (0~1 사이로 제한)
        lightIntensity[i] = saturate(dot(input.normal, input.lightPos[i]));
        // 광원 색상에 강도를 곱해 해당 광원의 색상 기여도 계산
        colorArray[i] = diffuseColor[i] * lightIntensity[i];
    }

    // 모든 광원의 기여도를 합산함
    colorSum = float4(0.0f, 0.0f, 0.0f, 1.0f);

    for (i = 0; i < 4; i++)
    {
        colorSum.r += colorArray[i].r;
        colorSum.g += colorArray[i].g;
        colorSum.b += colorArray[i].b;
    }

    // 합산된 조명 값에 텍스처 색상을 곱함 (0~1 범위를 넘지 않도록 saturate 적용)
    color = saturate(colorSum) * textureColor;

    return color;
}