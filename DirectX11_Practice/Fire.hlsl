////////////////////////////////////////////////////////////////////////////////
// 파일명: fire.hlsl
// 설명: 여러 개의 노이즈 텍스처 좌표 스크롤과 왜곡(Distortion)을 이용해 
//       동적인 화염(Fire) 효과를 표현하는 셰이더입니다.
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

// 버텍스 셰이더를 위해 매 프레임 계산에 필요한 값들을 담는 새로운 상수 버퍼를 생성합니다.
// 1. frameTime: 서로 다른 노이즈 텍스처를 스크롤하는 데 사용되는 증가하는 시간 값으로 매 프레임 업데이트됩니다.
// 2. scrollSpeeds: 3개의 서로 다른 스크롤 속도를 담는 3-float 배열입니다.
//    - x: 첫 번째 노이즈 텍스처의 스크롤 속도
//    - y: 두 번째 노이즈 텍스처의 스크롤 속도
//    - z: 세 번째 노이즈 텍스처의 스크롤 속도
// 3. scales: 3개의 서로 다른 노이즈 텍스처의 축척(스케일/옥타브)을 담는 3-float 배열입니다.
//    - x, y, z 값은 일반적으로 1, 2, 3으로 설정되어 첫 번째는 1회, 두 번째는 2회, 세 번째는 3회 타일링되도록 만듭니다.
// 4. padding: NoiseBuffer의 크기를 16의 배수로 맞추기 위한 단일 float 패딩 변수입니다. (일반적으로 0.0f 설정)
cbuffer NoiseBuffer
{
    float frameTime;
    float3 scrollSpeeds;
    float3 scales;
    float padding; // 16바이트 정렬을 위한 패딩
};

// 픽셀 셰이더에는 화염 효과를 생성하는 데 필요한 값들을 담는 DistortionBuffer 상수 버퍼가 추가됩니다.
// 3개의 왜곡(distortion) 배열은 개별 x, y 파라미터로 3개의 노이즈 텍스처를 왜곡하기 위한 x, y 값을 가집니다.
// distortionScale 및 distortionBias 값은 최종 결합된 노이즈 텍스처를 변형하여 불꽃 모양을 형성하는 데 사용됩니다.
cbuffer DistortionBuffer
{
    float2 distortion1;
    float2 distortion2;
    float2 distortion3;
    float distortionScale;
    float distortionBias;
};

// 불꽃 효과를 위한 3개의 텍스처:
// 1. fireTexture: 화염 색상 텍스처
// 2. noiseTexture: 노이즈 텍스처
// 3. alphaTexture: 알파(투명도) 텍스처
Texture2D fireTexture : register(t0);
Texture2D noiseTexture : register(t1);
Texture2D alphaTexture : register(t2);

// 샘플러 상태:
// 첫 번째 Wrap 샘플러 외에도 Clamp를 사용하는 두 번째 샘플러 상태를 추가합니다.
// Wrap을 사용할 경우 불꽃이 가장자리에서 넘어가는 현상(wrapping)이 발생하여 효과가 깨질 수 있습니다.
SamplerState SampleTypeWrap : register(s0);
SamplerState SampleTypeClamp : register(s1);


//////////////
// 타입 정의 (TYPEDEFS)
//////////////

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

// PixelInputType 구조체는 3개의 서로 다른 텍스처 좌표를 받도록 수정되었습니다.
// 단일 노이즈 텍스처 하나로부터 사실상 3개의 서로 다른 노이즈 텍스처를 생성하기 위해 동일한 텍스처를 3가지 방식으로 샘플링합니다.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 texCoords1 : TEXCOORD1;
    float2 texCoords2 : TEXCOORD2;
    float2 texCoords3 : TEXCOORD3;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType FireVertexShader(VertexInputType input)
{
    PixelInputType output;
	
    // 올바른 행렬 계산을 위해 위치 벡터의 w를 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 월드, 뷰, 투영 행렬을 사용하여 버텍스의 위치를 계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 기본 텍스처 좌표를 저장합니다.
    output.tex = input.tex;

    // 여기서 3개의 서로 다른 텍스처 샘플링 값을 생성하여
    // 동일한 노이즈 텍스처를 3개의 노이즈 텍스처처럼 활용합니다.
    // 각 텍스처 좌표에 대해 먼저 scales 배열 값에 따라 스케일을 적용(타일링)하고, 
    // frameTime 및 scrollSpeeds 배열 값을 사용하여 y 좌표를 위쪽으로 스크롤시킵니다. 
    // 3개 좌표의 스크롤 속도가 서로 달라 불꽃에 입체감을 제공합니다.

    // 첫 번째 스케일과 위쪽 스크롤 속도 값을 사용하여 첫 번째 노이즈 텍스처 좌표를 계산합니다.
    output.texCoords1 = (input.tex * scales.x);
    output.texCoords1.y = output.texCoords1.y + (frameTime * scrollSpeeds.x);

    // 두 번째 스케일과 위쪽 스크롤 속도 값을 사용하여 두 번째 노이즈 텍스처 좌표를 계산합니다.
    output.texCoords2 = (input.tex * scales.y);
    output.texCoords2.y = output.texCoords2.y + (frameTime * scrollSpeeds.y);

    // 세 번째 스케일과 위쪽 스크롤 속도 값을 사용하여 세 번째 노이즈 텍스처 좌표를 계산합니다.
    output.texCoords3 = (input.tex * scales.z);
    output.texCoords3.y = output.texCoords3.y + (frameTime * scrollSpeeds.z);
	
    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 FirePixelShader(PixelInputType input) : SV_TARGET
{
    float4 noise1;
    float4 noise2;
    float4 noise3;
    float4 finalNoise;
    float perturb;
    float2 noiseCoords;
    float4 fireColor;
    float4 alphaColor;

    // 먼저 노이즈 텍스처를 3가지 방식으로 샘플링하여 3개의 서로 다른 노이즈 값을 생성합니다.
    // 그 후 텍스처 픽셀 값을 (-1, +1) 범위로 이동시킵니다.
    
    // 3개의 서로 다른 텍스처 좌표로 동일한 노이즈 텍스처를 샘플링하여 3개의 노이즈 스케일을 얻습니다.
    noise1 = noiseTexture.Sample(SampleTypeWrap, input.texCoords1);
    noise2 = noiseTexture.Sample(SampleTypeWrap, input.texCoords2);
    noise3 = noiseTexture.Sample(SampleTypeWrap, input.texCoords3);

    // 노이즈 범위를 (0, 1)에서 (-1, +1)로 변환합니다.
    noise1 = (noise1 - 0.5f) * 2.0f;
    noise2 = (noise2 - 0.5f) * 2.0f;
    noise3 = (noise3 - 0.5f) * 2.0f;

    // 이제 왜곡 양(distortion amount)에 따라 x, y 샘플링 좌표를 축소시킵니다.
    // 왜곡이 적용된 후 3개의 텍스처 값이 하나의 결합된 노이즈 값으로 합쳐져 해당 픽셀의 최종 노이즈를 형성합니다.

    // 3개의 노이즈 x, y 좌표를 3개의 개별 왜곡 x, y 값으로 왜곡합니다.
    noise1.xy = noise1.xy * distortion1.xy;
    noise2.xy = noise2.xy * distortion2.xy;
    noise3.xy = noise3.xy * distortion3.xy;

    // 왜곡된 3개의 노이즈 결과를 하나로 결합합니다.
    finalNoise = noise1 + noise2 + noise3;

    // 이제 최종 노이즈 결과에 섭동(perturbation, 왜곡)을 적용하여 전체적인 노이즈 텍스처에 불꽃 느낌을 만듭니다.
    // 위쪽으로 갈수록 변형을 크게 주고, 아래쪽으로 갈수록 변형을 줄입니다. 
    // 이를 통해 위쪽에는 일렁이는 불꽃 효과를 만들고 아래쪽에는 단단한 불꽃 기둥 모양을 형성합니다.

    // distortionScale 및 distortionBias 값을 사용하여 입력 텍스처 Y 좌표를 섭동시킵니다.
    // 텍스처 위쪽으로 올라갈수록 섭동이 강해져 윗부분이 일렁이는 불꽃 효과를 만듭니다.
    perturb = ((1.0f - input.tex.y) * distortionScale) + distortionBias;

    // 이제 화염 색상 텍스처를 샘플링하는 데 사용될 왜곡 및 변형된 텍스처 샘플링 좌표를 생성합니다.
    noiseCoords.xy = (finalNoise.xy * perturb) + input.tex.xy;

    // 왜곡된 노이즈 샘플링 좌표를 사용하여 화염 색상 텍스처와 알파 텍스처를 모두 샘플링하여 화염 효과를 생성합니다.
    
    // 왜곡/변형된 샘플링 좌표를 사용하여 화염 텍스처에서 색상을 샘플링합니다.
    // 불꽃이 텍스처 가장자리로 잘려 넘어가는 현상을 방지하기 위해 Wrap 대신 Clamping 샘플러를 사용합니다.
    fireColor = fireTexture.Sample(SampleTypeClamp, noiseCoords.xy);

    // 왜곡/변형된 샘플링 좌표를 사용하여 알파 텍스처에서 알파 값을 샘플링합니다. (불꽃의 투명도 제어)
    // 마찬가지로 불꽃이 넘쳐나는 것을 방지하기 위해 Clamping 샘플러를 사용합니다.
    alphaColor = alphaTexture.Sample(SampleTypeClamp, noiseCoords.xy);

    // 알파 값과 화염 색상을 결합하여 블렌딩된 최종 투명 불꽃 효과를 생성합니다.
    
    // 불꽃의 알파 블렌딩을 왜곡/변형된 알파 텍스처 값으로 설정합니다.
    fireColor.a = alphaColor.r; // (참고: alphaColor 채널 값을 알파에 할당)
    fireColor.rgb *= fireColor.a; // 알파 값에 따라 RGB 색상을 조정하여 불꽃의 투명도를 적용합니다.
    return fireColor;
}