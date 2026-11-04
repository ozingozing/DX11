////////////////////////////////////////////////////////////////////////////////
// 파일명: refraction.hlsl
// 설명: 굴절(Refraction) 셰이더는 라이트 셰이더에 클립 평면(Clip Plane)을 추가하여 재작성된 셰이더입니다.
//       굴절은 씬을 정상적으로 렌더링하되 물 아래쪽에 있는 객체만 렌더링하므로, 
//       라이트 셰이더에 클립 평면을 함께 사용하여 이 효과를 구현합니다.
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

// 라이트 셰이더를 굴절 셰이더로 만들기 위해 클립 평면 버퍼를 추가합니다.
cbuffer ClipPlaneBuffer
{
    float4 clipPlane;
};

// 조명 버퍼
cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float padding; // 16바이트 정렬을 위한 패딩
};

// 샘플러 상태 및 텍스처
SamplerState SampleType : register(s0);
Texture2D shaderTexture : register(t0);


//////////////
// 타입 정의 (TYPEDEFS)
//////////////

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// 굴절 셰이더에 필요한 클리핑 효과를 얻기 위해 픽셀 입력 구조체에 클립 거리 변수를 추가합니다.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float clip : SV_ClipDistance0;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType RefractionVertexShader(VertexInputType input)
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
    
    // 월드 행렬에 대해서만 노멀 벡터를 계산합니다.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    // 노멀 벡터를 정규화합니다.
    output.normal = normalize(output.normal);

    // 여기에서 클리핑 평면을 설정합니다.
    // 이를 제외한 셰이더의 나머지 부분은 기존 라이트 버텍스 셰이더와 동일합니다.
    // 클리핑 평면 설정
    output.clip = dot(mul(input.position, worldMatrix), clipPlane);

    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
// 굴절 픽셀 셰이더는 기존 라이트 픽셀 셰이더와 완전히 동일합니다. 
// 버텍스 셰이더에서 이미 클리핑이 구현되었기 때문에 굴절 효과를 위해 픽셀 셰이더 코드를 수정할 필요가 없습니다. 
// SV_ClipDistance0를 포함하도록 PixelInputType만 업데이트합니다.
float4 RefractionPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // 해당 위치의 텍스처 픽셀을 샘플링합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
	
    // 모든 픽셀의 기본 출력 색상을 환경광(Ambient Light) 값으로 설정합니다.
    color = ambientColor;

    // 계산을 위해 광원 방향을 반전시킵니다.
    lightDir = -lightDirection;

    // 이 픽셀에 도달하는 광량(Light Intensity)을 계산합니다.
    lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        // 난반사광(Diffuse Color) 색상과 빛의 세기를 바탕으로 최종 난반사 색상을 결정합니다.
        color += (diffuseColor * lightIntensity);
    }

    // 최종 조명 색상을 0.0 ~ 1.0 사이로 클램핑(Saturate)합니다.
    color = saturate(color);

    // 텍스처 픽셀과 입력 색상을 곱하여 최종 결과를 구합니다.
    color = color * textureColor;
	
    return color;
}