////////////////////////////////////////////////////////////////////////////////
// 파일명: glass.hlsl
// 설명: 유리를 통과하는 빛의 굴절(Refraction) 및 질감 효과를 표현하는 셰이더입니다.
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

// GlassBuffer는 refractionScale을 설정하는 데 사용됩니다.
// refractionScale 변수는 굴절 텍스처의 섭동(Perturbation, 왜곡) 정도를 조절하는 데 사용됩니다.
// 일반적으로 유리에는 낮은 값을 사용하고, 얼음에는 더 높은 값을 적용합니다.
cbuffer GlassBuffer
{
    float refractionScale;
    float translation;
    float2 padding; // 16바이트 정렬을 위한 패딩
};

// 샘플러 상태
SamplerState SampleType : register(s0);

// 유리 셰이더는 3가지 서로 다른 텍스처를 사용합니다:
// 1. colorTexture: 유리에 사용되는 기본 표면 색상입니다.
// 2. normalTexture: 모든 노멀 벡터를 포함하는 노멀 맵 룩업 테이블입니다.
// 3. refractionTexture: 2D 텍스처로 렌더링된 유리 뒤쪽의 3D 씬입니다.
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D refractionTexture : register(t2);


//////////////
// 타입 정의 (TYPEDEFS)
//////////////

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

// PixelInputType 구조체에는 픽셀 셰이더로 전달될 굴절 버텍스 좌표용 refractionPosition 변수가 추가되어 있습니다.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 refractionPosition : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType GlassVertexShader(VertexInputType input)
{
    PixelInputType output;
    matrix viewProjectWorld;
	
    // 올바른 행렬 계산을 위해 위치 벡터의 w를 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 월드, 뷰, 투영 행렬을 사용하여 버텍스의 위치를 계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스처 좌표를 저장합니다.
    output.tex = input.tex;

    // 입력 버텍스 좌표를 투영 좌표로 변환하는 데 사용할 행렬을 생성합니다.
    
    // 굴절을 위한 뷰-투영-월드 행렬을 생성합니다.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);

    // 입력 버텍스 좌표를 투영된 값으로 변환하고 픽셀 셰이더로 전달합니다.
    
    // viewProjectWorld 행렬에 대해 입력 위치를 계산합니다.
    output.refractionPosition = mul(input.position, viewProjectWorld);
	
    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 GlassPixelShader(PixelInputType input) : SV_TARGET
{
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 refractionColor;
    float4 textureColor;
    float4 color;

    // 먼저 입력된 투영 동종 좌표(-1, +1)를 (0, 1) 텍스처 좌표로 변환합니다.
    
    // 투영된 굴절 텍스처 좌표를 계산합니다.
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // 다음으로 노멀 맵을 샘플링하고 (0, 1) 텍스처 좌표 범위를 (-1, 1) 좌표 범위로 이동합니다.
    
    //float2 normalTexCoord = input.tex;
    //normalTexCoord.y -= translation;
    
    // 노멀 맵 텍스처에서 노멀을 샘플링합니다.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // 노멀의 범위를 (0, 1)에서 (-1, +1)로 확장합니다.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // 이제 계산된 노멀 값을 이용하여 굴절 텍스처 샘플링 위치를 흔듭니다(Perturb). 
    // 또한 노멀에 refractionScale을 곱해 왜곡의 정도를 증가시키거나 감소시킵니다.
    
    // 유리를 통한 빛의 왜곡을 시뮬레이션하기 위해 노멀 맵 값으로 텍스처 좌표 샘플링 위치를 재조정합니다.
    refractTexCoord = refractTexCoord + (normal.xy * refractionScale);

    // 왜곡된 좌표를 사용하여 굴절 텍스처에서 텍스처 픽셀을 샘플링하고, 일반 입력 텍스처 좌표를 사용하여 색상 텍스처를 샘플링합니다.
    
    // 왜곡된 텍스처 좌표를 사용하여 굴절 텍스처에서 텍스처 픽셀을 샘플링합니다.
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    // 유리 색상 텍스처에서 텍스처 픽셀을 샘플링합니다.
    textureColor = colorTexture.Sample(SampleType, input.tex);

    // 마지막으로 굴절 텍스처와 색상 텍스처를 결합하여 최종 결과를 구합니다.
    
    // 최종 색상을 위해 유리 색상과 굴절 값을 동등하게(50:50) 결합합니다.
    color = lerp(refractionColor, textureColor, 0.5f);

    return color;
}