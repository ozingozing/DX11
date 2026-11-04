////////////////////////////////////////////////////////////////////////////////
// 파일명: water.hlsl
// 설명: 반사, 굴절, 노멀 맵을 결합하여 수면(Water) 효과를 표현하는 셰이더입니다.
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

// 반사 튜토리얼과 마찬가지로 물 셰이더에는 반사 행렬이 필요합니다.
cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

// 수면 이동 변수(waterTranslation)는 매 프레임
// 텍스처 샘플링 좌표를 이동시켜 물의 움직임을 시뮬레이션하는 데 사용됩니다.
// reflectRefractScale 변수는 노멀 맵에 대한 물결 파동의 크기를 제어하는 데 사용됩니다.
// 노멀 맵마다 노멀의 높낮이 변화 폭이 다를 수 있으므로, 이를 제어하는 변수를 두는 것이 매우 유용합니다.
cbuffer WaterBuffer
{
    float waterTranslation;
    float reflectRefractScale;
    float2 padding; // 16바이트 정렬을 위한 패딩
    float3 cameraPosition; // 카메라 위치
    float padding2; // 16바이트 정렬을 위한 패딩
};

// 샘플러 상태
SamplerState SampleType : register(s0);

// 물 셰이더에는 3개의 텍스처가 필요합니다:
// 1. 씬 반사를 위한 반사 텍스처 (reflectionTexture)
// 2. 씬 굴절을 위한 굴절 텍스처 (refractionTexture)
// 3. 물결 파동을 시뮬레이션하기 위한 노멀 맵 텍스처 (normalTexture)
Texture2D reflectionTexture : register(t0);
Texture2D refractionTexture : register(t1);
Texture2D normalTexture : register(t2);


//////////////
// 타입 정의 (TYPEDEFS)
//////////////

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

// PixelInputType에는 반사 텍스처 좌표와 굴절 텍스처 좌표를 위한 2개의 추가 텍스처 좌표 입력이 포함됩니다.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
    float3 worldPosition : TEXCOORD3;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType WaterVertexShader(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
    matrix viewProjectWorld;
	
    // 올바른 행렬 계산을 위해 위치 벡터의 w를 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 월드, 뷰, 투영 행렬을 사용하여 버텍스의 위치를 계산합니다.
    output.position = mul(input.position, worldMatrix);
    // 월드 공간에서의 위치를 저장합니다.
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스처 좌표를 저장합니다.
    output.tex = input.tex;

    // 반사 튜토리얼과 마찬가지로 반사-투영-월드 행렬을 생성하고 이로부터 반사 좌표를 계산합니다.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    // reflectProjectWorld 행렬을 사용하여 입력 위치를 계산합니다.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);

    // 굴절 좌표는 뷰-투영-월드 행렬을 사용한다는 점을 제외하면 반사 좌표와 동일한 방식으로 계산됩니다.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);
   
    // viewProjectWorld 행렬을 사용하여 입력 위치를 계산합니다.
    output.refractionPosition = mul(input.position, viewProjectWorld);
	
    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 WaterPixelShader(PixelInputType input) : SV_TARGET
{
    float2 reflectTexCoord;
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 reflectionColor;
    float4 refractionColor;
    float4 color;

    // 평행이동(Translate) 셰이더 튜토리얼과 마찬가지로, 매 프레임 업데이트되는 이동 변수를 사용하여 
    // 물 노멀 맵 텍스처를 Y축으로 이동시켜 움직임을 시뮬레이션합니다.
    input.tex.y += waterTranslation;

    // 반사 및 굴절 좌표를 모두 (0, 1) 범위의 텍스처 좌표로 변환합니다.
    // 투영된 반사 텍스처 좌표 계산
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	
    // 투영된 굴절 텍스처 좌표 계산
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // 노멀 맵에서 해당 픽셀의 노멀을 샘플링하고 범위를 (-1, +1)로 확장합니다.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // 노멀의 범위를 (0, 1)에서 (-1, +1)로 확장합니다.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // 이제 노멀 맵 값을 사용하여 반사 및 굴절 좌표를 왜곡합니다. 
    // 실제 파도가 빛을 왜곡하는 것처럼, -1에서 +1로 변화하는 노멀을 이용해 시야를 왜곡함으로써 물결 파동 효과를 만듭니다.
    // 물결이 너무 튀지 않고 자연스럽게 보이도록 노멀 맵 값에 reflectRefractScale을 곱해줍니다.
    reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
    refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);

    // 업데이트된 텍스처 샘플링 좌표를 사용하여 반사 및 굴절 픽셀을 샘플링합니다.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    float3 viewDirection = normalize(cameraPosition - input.worldPosition);
    float fresnel = saturate(dot(viewDirection, float3(0.f, 1.f, 0.f)));
    
    // 선형 보간(lerp)을 사용하여 반사 결과와 굴절 결과를 최종 색상으로 결합합니다.
    color = lerp(reflectionColor, refractionColor, fresnel);
	
    return color;
}