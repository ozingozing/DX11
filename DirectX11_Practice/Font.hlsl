////////////////////////////////////////////////////////////////////////////////
// 파일명: Font.hlsl
// 목적: 폰트 렌더링을 위한 정점 및 픽셀 셰이더
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////

// 1. 행렬 버퍼 (Vertex Shader 전용)
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 2. 픽셀 컬러 버퍼 (Pixel Shader 전용)
// 폰트의 색상을 외부(CPU)에서 제어하기 위해 사용됩니다.
cbuffer PixelBuffer : register(b0)
{
    float4 pixelColor;
};

// 텍스처 및 샘플러 설정
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


//////////////
// TYPEDEFS //
//////////////

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (FontVertexShader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType FontVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 행렬 연산을 위해 w 성분을 1.0으로 명시합니다.
    input.position.w = 1.0f;

    // 정점 위치를 월드, 뷰, 투영 행렬과 연산하여 최종 화면 좌표로 변환합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 픽셀 셰이더로 전달할 텍스처 좌표를 저장합니다.
    output.tex = input.tex;

    return output;
}


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (FontPixelShader)
////////////////////////////////////////////////////////////////////////////////
float4 FontPixelShader(PixelInputType input) : SV_TARGET
{
    float4 color;

    // 현재 위치의 텍스처 픽셀(Texel)을 샘플링합니다.
    color = shaderTexture.Sample(SampleType, input.tex);

    // [중요 로직] 투명도 처리
    // 폰트 이미지에서 검은색(R=0) 부분은 텍스트가 없는 배경이므로 투명하게 처리합니다.
    if (color.r == 0.0f)
    {
        color.a = 0.0f;
    }
    // 검은색이 아닌 부분은 실제 글자 픽셀입니다.
    else
    {
        // 알파 값을 1.0으로 고정하고, CPU에서 전달받은 pixelColor를 곱해 글자색을 입힙니다.
        color.a = 1.0f;
        color = color * pixelColor;
    }

    return color;
}