cbuffer MatrixBuffer        // b0
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer FogBuffer           // b1 (두 번째로 선언했으므로)
{
    float fogStart;
    float fogEnd;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float fogFactor : FOG; // ★ 새로 추가: 픽셀 셰이더로 넘길 안개 계수
};

PixelInputType FogVertexShader(VertexInputType input)
{
    PixelInputType output;
    input.position.w = 1.0f;

    // 평소처럼 월드 → 뷰 → 투영 변환
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = input.tex;

    // 뷰 공간 위치 = 카메라 기준 좌표. 이때 z가 카메라 앞쪽으로의 거리
    float4 viewPos = mul(input.position, worldMatrix);
    viewPos = mul(viewPos, viewMatrix);

    // 선형 안개 계수, 0~1로 자르기
    output.fogFactor = saturate((fogEnd - viewPos.z) / (fogEnd - fogStart));
    return output;
}

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

float4 FogPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    float4 fogColor = float4(0.5f, 0.5f, 0.5f, 1.0f); // 회색 안개

    // f = 1이면 텍스처색, f = 0이면 안개색
    return input.fogFactor * textureColor + (1.0f - input.fogFactor) * fogColor;
}