////////////////////////////////////////////////////////////////////////////////
// 파일명: transparent.hlsl
// 설명: 투명도(Alpha Blending) 효과를 처리하도록 수정된 텍스처 셰이더입니다.
////////////////////////////////////////////////////////////////////////////////

/////////////
// 전역 변수 (GLOBALS)
/////////////

// 행렬 버퍼: 월드, 뷰, 투영 행렬을 저장합니다.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 블렌드 양(blendAmount) 변수를 담을 새로운 상수 버퍼를 추가합니다. 
// blendAmount는 텍스처를 블렌딩할 백분율을 나타냅니다. 허용되는 값의 범위는 0에서 1 사이의 float 값입니다.
// 이 튜토리얼에서는 0.5로 설정하여 50%의 투명도를 의미하도록 합니다.
// (참고: 상수 버퍼의 16바이트 정렬 규칙을 맞추기 위해 float3 패딩을 추가합니다.)
cbuffer TransparentBuffer
{
    float blendAmount;
    float3 padding;
};

// 텍스처와 샘플러 상태
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


//////////////
// 타입 정의 (TYPEDEFS)
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
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
// 투명 텍스처 버텍스 셰이더는 일반 텍스처 버텍스 셰이더와 동일합니다.
PixelInputType TransparentVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 올바른 행렬 계산을 위해 위치 벡터의 w를 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 월드, 뷰, 투영 행렬을 사용하여 버텍스의 위치를 계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스처 좌표를 저장합니다.
    output.tex = input.tex;
    
    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
// 여기서는 blendAmount 변수를 사용하여 텍스처의 투명도를 설정합니다.
// 픽셀의 알파 값을 블렌드 양으로 설정하며, 렌더링될 때 설정한 블렌드 상태(Blend State)와 이 알파 값을 함께 사용하여 투명도 효과를 생성합니다.
float4 TransparentPixelShader(PixelInputType input) : SV_TARGET
{
    float4 color;

    // 해당 위치의 텍스처 픽셀을 샘플링합니다.
    color = shaderTexture.Sample(SampleType, input.tex);

    // 알파 블렌딩 효과를 만들기 위해 이 픽셀의 알파 값을 블렌딩 양으로 설정합니다.
    color.a = blendAmount;

    return color;
}