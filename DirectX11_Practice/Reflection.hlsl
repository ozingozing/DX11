////////////////////////////////////////////////////////////////////////////////
// 파일명: reflection.hlsl
// 설명: 반사(Reflection) 효과를 처리하도록 수정된 텍스처 셰이더입니다.
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

// 반사 행렬(reflectionMatrix)을 담을 새로운 상수 버퍼를 추가합니다.
cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

// 기본 텍스처
Texture2D shaderTexture : register(t0);

// 씬 반사 렌더 투 텍스처를 위한 새로운 텍스처 변수를 추가합니다.
Texture2D reflectionTexture : register(t1);

// 샘플러 상태
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
    // PixelInputType은 이제 투영된 반사 텍스처 입력 위치를 저장하는 데 사용될 
    // reflectionPosition이라는 4-float 텍스처 좌표 변수를 가집니다.
    float4 reflectionPosition : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType ReflectionVertexShader(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;

    // 올바른 행렬 계산을 위해 위치 벡터의 w를 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 월드, 뷰, 투영 행렬을 사용하여 버텍스의 위치를 계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스처 좌표를 저장합니다.
    output.tex = input.tex;

    // 버텍스 셰이더의 첫 번째 변경 사항은 입력 위치 값을 투영된 반사 위치로 변환하기 위한 행렬을 생성하는 것입니다.
    // 이 행렬은 반사 행렬, 투영 행렬, 그리고 월드 행렬의 조합입니다.
    
    // 반사-투영-월드 행렬을 생성합니다.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    // 이제 입력 위치를 투영된 반사 위치로 변환합니다. 
    // 이 변환된 위치 좌표는 픽셀 셰이더에서 투영된 반사 텍스처를 매핑할 위치를 유추하는 데 사용됩니다.
    
    // reflectProjectWorld 행렬을 사용하여 입력 위치를 계산합니다.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);

    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 ReflectionPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float2 reflectTexCoord;
    float4 reflectionColor;
    float4 color;

    // 해당 위치의 텍스처 픽셀을 샘플링합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // 입력된 반사 위치의 동종(homogeneous) 좌표를 올바른 텍스처 좌표로 변환해야 합니다.
    // 이를 위해 먼저 W 좌표로 나눕니다. 그러면 -1에서 +1 범위의 tu 및 tv 좌표가 남게 되며, 
    // 이를 0에서 +1 범위로 맞추기 위해 2로 나누고 0.5를 더합니다.
    
    // 투영된 반사 텍스처 좌표를 계산합니다.
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

    // 이제 반사 텍스처에서 샘플링할 때, 변환된 투영 반사 좌표를 사용하여 이 투영 반사 위치에 맞는 올바른 반사 픽셀을 얻습니다.
    
    // 투영된 텍스처 좌표를 사용하여 반사 텍스처에서 텍스처 픽셀을 샘플링합니다.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);

    // 마지막으로, 바닥의 텍스처와 반사 텍스처를 블렌딩하여 바닥에 반사된 큐브 효과를 만듭니다.
    // 여기서는 0.15의 팩터(factor)로 두 텍스처 간의 선형 보간(linear interpolation)을 사용합니다.
    // 더 강하거나 다른 효과를 원한다면 이를 다른 블렌드 방정식으로 바꾸거나 팩터 값을 변경할 수 있습니다.
    
    // 블렌드 효과를 위해 두 텍스처 간 선형 보간을 수행합니다.
    color = lerp(textureColor, reflectionColor, 0.5f);

    return color;
}