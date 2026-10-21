/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

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

// textureTranslation이라는 float 변수를 포함하는 새로운 버퍼를 추가합니다. 
// 이 값은 텍스처 평행이동 위치를 업데이트하기 위해 applicationclass.cpp의 Render 호출 중에 설정됩니다. 
// 여기에 설정되는 값은 0과 1 사이입니다.
cbuffer TranslationBuffer
{
    float textureTranslation;
    float3 padding; // 16바이트 정렬을 위해 추가된 패딩
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TranslateVertexShader(VertexInputType input)
{
    PixelInputType output;


    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
    return output;
}

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더는 textureTranslation 만큼 텍스처 좌표를 이동(Translate)시키는 작업을 수행하는 곳입니다.
// 입력된 텍스처 좌표에 각 픽셀 셰이더 호출마다 이동량을 더해줍니다. 
// 이렇게 하면 다각형 면 전체에 걸쳐 텍스처가 균일하게 평행이동하게 됩니다.
float4 TranslatePixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

    // 픽셀을 샘플링할 위치를 평행이동(변환)시킵니다.
    input.tex.x += textureTranslation;

    // 해당 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처의 픽셀 색상을 샘플링합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}