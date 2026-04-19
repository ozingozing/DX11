/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix; // 월드 행렬
    matrix viewMatrix; // 뷰 행렬
    matrix projectionMatrix; // 투영 행렬
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// 멀티텍스처 셰이더는 두 개의 텍스처를 사용합니다.
// t0 슬롯에는 기본이 되는 텍스처(예: stone01.tga)를,
// t1 슬롯에는 혼합할 텍스처(예: dirt01.tga)를 할당합니다.
Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
SamplerState SampleType : register(s0);

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
PixelInputType MultiTextureVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 행렬 연산을 위해 위치 벡터의 w 성분을 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 정점의 위치를 월드, 뷰, 투영 행렬과 계산하여 변환합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스처 좌표를 전달합니다.
    output.tex = input.tex;
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (픽셀 셰이더)
////////////////////////////////////////////////////////////////////////////////
float4 MultiTexturePixelShader(PixelInputType input) : SV_TARGET
{
    float4 color1;
    float4 color2;
    float4 blendColor;

    // 샘플러를 사용하여 해당 텍스처 좌표 위치의 픽셀 색상을 각각 추출합니다.
    color1 = shaderTexture1.Sample(SampleType, input.tex);
    color2 = shaderTexture2.Sample(SampleType, input.tex);

    // 픽셀 셰이더에서 돌(stone)과 흙(dirt) 텍스처를 샘플링한 후, 
    // 튜토리얼 초반에 다루었던 혼합 공식을 사용하여 두 픽셀을 결합합니다.
    
    // 두 텍스처를 결합합니다 (곱하기 연산 후 2배 밝게 보정).
    blendColor = color1 * color2 * 2.0;

    // 모든 색상 채널의 값이 0과 1 사이에 머물도록 최종 결과값을 제한(Saturate)합니다.
    blendColor = saturate(blendColor);

    return blendColor;
}