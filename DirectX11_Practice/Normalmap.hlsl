/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;      // 월드 행렬
    matrix viewMatrix;       // 뷰 행렬
    matrix projectionMatrix; // 투영 행렬
};

cbuffer LightBuffer : register(b1)
{
    float4 diffuseColor;     // 빛의 색상
    float3 lightDirection;   // 빛의 방향
    float padding;           // 16바이트 정렬을 위한 패딩
};

// 텍스처 및 샘플러 설정
Texture2D shaderTexture1 : register(t0); // Base Color Map
Texture2D shaderTexture2 : register(t1); // Normal Map
SamplerState SampleType : register(s0);

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;     // 법선 (N)
    float3 tangent : TANGENT;   // 접선 (T)
    float3 binormal : BINORMAL; // 종법선 (B)
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
PixelInputType NormalMapVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 행렬 계산을 위해 w 성분을 1.0으로 설정
    input.position.w = 1.0f;

    // 정점의 위치를 월드, 뷰, 투영 행렬로 변환
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더로 텍스처 좌표 전달
    output.tex = input.tex;
    
    /* [TBN 벡터의 월드 변환]
       노멀 맵 연산은 월드 공간에서 이루어져야 하므로, 
       C++에서 계산된 T, B, N 벡터를 월드 행렬과 곱해 방향을 회전시킵니다.
    */
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    output.tangent = mul(input.tangent, (float3x3)worldMatrix);
    output.tangent = normalize(output.tangent);

    output.binormal = mul(input.binormal, (float3x3)worldMatrix);
    output.binormal = normalize(output.binormal);

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (픽셀 셰이더)
////////////////////////////////////////////////////////////////////////////////
float4 NormalMapPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // 1. 컬러 텍스처와 노멀 맵에서 각각 샘플링
    textureColor = shaderTexture1.Sample(SampleType, input.tex);
    bumpMap = shaderTexture2.Sample(SampleType, input.tex);

    /* 2. 노멀 값 범위 확장 (Unpacking)
       텍스처의 [0, 1] 범위를 실제 벡터 방향인 [-1, 1] 범위로 변환합니다.
    */
    bumpMap = (bumpMap * 2.0f) - 1.0f;

    /* 3. 최종 법선 조립 (The Core of Normal Mapping)
       노멀 맵의 데이터(x, y, z)를 가중치로 삼아 
       기준 축인 Tangent, Binormal, Normal 벡터를 섞어 새로운 법선을 생성합니다.
    */
    bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);

    // 조립된 법선을 다시 정규화
    bumpNormal = normalize(bumpNormal);

    // 4. 조명 계산을 위해 광원 방향 반전 (Light Vector가 정점을 향하도록)
    lightDir = -lightDirection;

    // 5. 램버트 코사인 법칙(Dot Product)을 이용한 조명 강도 계산
    // 조립된 bumpNormal을 사용함으로써 픽셀마다 다른 반사광이 생깁니다.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // 6. 조명 강도와 조명 색상을 결합
    color = saturate(diffuseColor * lightIntensity);

    // 7. 최종 조명 결과에 텍스처 색상을 곱함
    color = color * textureColor;

    return color;
}