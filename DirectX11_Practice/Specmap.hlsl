////////////////////////////////////////////////////////////////////////////////
// Filename: Specmap.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// 전역 변수 //
/////////////

// 스페큘러 맵 셰이더는 총 3개의 텍스처를 사용한다.
// t0 : 기본 색상 텍스처
// t1 : 노멀 맵 텍스처
// t2 : 스페큘러 맵 텍스처
Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);

// 텍스처 샘플링 방식
SamplerState SampleType : register(s0);

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float padding;
};

// 조명 관련 상수 버퍼
cbuffer LightBuffer
{
    float4 diffuseColor; // 확산광 색상
    float4 specularColor; // 스페큘러 색상
    float specularPower; // 스페큘러 강도 지수
    float3 lightDirection; // 빛의 방향
};


//////////////
// 구조체 정의 //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;

    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;

    float3 viewDirection : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType SpecMapVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    // 행렬 연산을 위해 정점 위치의 w 값을 1로 설정한다.
    // 위치 벡터는 float4(x, y, z, 1) 형태여야 이동 변환까지 적용된다.
    input.position.w = 1.0f;

    // 정점 위치를 월드 공간으로 변환한다.
    output.position = mul(input.position, worldMatrix);

    // 월드 공간 위치를 뷰 공간으로 변환한다.
    output.position = mul(output.position, viewMatrix);

    // 뷰 공간 위치를 투영 공간으로 변환한다.
    // 최종적으로 SV_POSITION으로 전달되어 화면에 그려질 위치가 된다.
    output.position = mul(output.position, projectionMatrix);
    
    // 텍스처 좌표를 픽셀 셰이더로 전달한다.
    // 픽셀 셰이더는 이 좌표로 기본 텍스처, 노멀 맵, 스페큘러 맵을 샘플링한다.
    output.tex = input.tex;
    
    // 법선 벡터를 월드 행렬로 변환한다.
    // 방향 벡터이므로 위치처럼 view/projection까지 곱하지 않고 월드 공간까지만 변환한다.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal); // 변환 과정에서 길이가 달라질 수 있으므로 정규화한다.

    // 탄젠트 벡터를 월드 공간으로 변환한다.
    // 노멀 맵의 x 방향 성분을 월드 공간 방향으로 바꾸기 위해 필요하다.
    output.tangent = mul(input.tangent, (float3x3) worldMatrix);
    output.tangent = normalize(output.tangent); // 탄젠트 벡터를 정규화한다.

    // 바이노멀/비탄젠트 벡터를 월드 공간으로 변환한다.
    // 노멀 맵의 y 방향 성분을 월드 공간 방향으로 바꾸기 위해 필요하다.
    output.binormal = mul(input.binormal, (float3x3) worldMatrix);
    output.binormal = normalize(output.binormal); // 바이노멀 벡터를 정규화한다.

    // 현재 정점의 월드 공간 위치를 계산한다.
    // 카메라 방향 벡터를 구할 때 사용한다.
    worldPosition = mul(input.position, worldMatrix);

    // 카메라 위치에서 정점의 월드 위치를 빼서,
    // 현재 정점에서 카메라를 향하는 방향 벡터를 구한다.
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
	
    // 시선 방향 벡터를 정규화한다.
    // 픽셀 셰이더에서 스페큘러 계산에 사용된다.
    output.viewDirection = normalize(output.viewDirection);

    // 픽셀 셰이더로 변환된 위치, 텍스처 좌표, TBN 벡터, 시선 방향을 넘긴다.
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더
////////////////////////////////////////////////////////////////////////////////

float4 SpecMapPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float4 specularIntensity;
    float3 reflection;
    float4 specular;

    // 현재 픽셀 위치의 기본 색상 텍스처를 샘플링한다.
    textureColor = shaderTexture1.Sample(SampleType, input.tex);

    // 현재 픽셀 위치의 노멀 맵 텍스처를 샘플링한다.
    bumpMap = shaderTexture2.Sample(SampleType, input.tex);

    // 노멀 맵 값을 (0 ~ 1) 범위에서 (-1 ~ 1) 범위로 변환한다.
    bumpMap = (bumpMap * 2.0f) - 1.0f;

    // 노멀 맵의 값을 tangent, binormal, normal 벡터에 적용하여 실제 범프 노멀을 계산한다.
    bumpNormal = (bumpMap.x * input.tangent) +
                 (bumpMap.y * input.binormal) +
                 (bumpMap.z * input.normal);

    // 계산된 범프 노멀을 정규화한다.
    bumpNormal = normalize(bumpNormal);

    // 조명 계산을 위해 빛의 방향을 반대로 뒤집는다.
    lightDir = -lightDirection;

    // 범프 노멀과 빛의 방향을 이용해 현재 픽셀에 닿는 빛의 세기를 계산한다.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // 확산광 색상과 빛의 세기를 곱해 최종 확산광 색상을 계산한다.
    color = saturate(diffuseColor * lightIntensity);

    // 확산광 결과에 기본 색상 텍스처를 곱한다.
    color = color * textureColor;

    // 빛이 닿는 부분에만 스페큘러 계산을 수행한다.
    if (lightIntensity > 0.0f)
    {
        // 현재 픽셀 위치의 스페큘러 맵 텍스처를 샘플링한다.
        specularIntensity = shaderTexture3.Sample(SampleType, input.tex);

        // 노멀 맵으로 계산된 범프 노멀을 기준으로 반사 벡터를 계산한다.
        reflection = normalize(2.0f * lightIntensity * bumpNormal - lightDir);

        // 반사 벡터와 시선 방향을 이용해 스페큘러 값을 계산한다.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);

        // 스페큘러 색상과 스페큘러 맵의 강도를 적용한다.
        specular = specular * specularColor * specularIntensity;

        // 최종 색상에 스페큘러 값을 더한다.
        color = saturate(color + specular);
    }

    return color;
}