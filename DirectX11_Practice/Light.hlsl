/////////////
// GLOBALS //
/////////////

// 행렬 버퍼: 정점의 위치를 3D 공간으로 변환하기 위한 행렬들
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // 월드 변환 행렬
    matrix viewMatrix; // 뷰 변환 행렬
    matrix projectionMatrix; // 투영 변환 행렬
};

// 조명 버퍼: 광원의 색상과 방향 정보 (LightClass에서 설정됨)
cbuffer LightBuffer : register(b1)
{
    float4 ambientColor;
    float4 diffuseColor; // 확산광 색상
    float3 lightDirection; // 광원이 비추는 방향
    // 광 버퍼가 업데이트되어 반사광 계산에 필요한
    // specularColor 및 specularPower 값을 저장하게 되었습니다.
    float specularPower;
    float4 specularColor;
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float padding;
};

//////////////
// TYPEDEFS //
//////////////

// 정점 셰이더 입력 구조체
struct VertexInputType
{
    float4 position : POSITION; // 정점 위치
    float2 tex : TEXCOORD0; // 텍스처 좌표
    float3 normal : NORMAL; // 법선 벡터 (조명 계산용)
};

// 픽셀 셰이더 입력 구조체 (정점 셰이더의 출력)
// PixelInputType 구조체는 시점 방향을 정점 셰이더에서 계산한 다음
// 반사광 계산을 위해 픽셀 셰이더로 전달해야 하므로 수정됩니다.
struct PixelInputType
{
    float4 position : SV_POSITION; // 화면상의 투영된 위치
    float2 tex : TEXCOORD0; // 보간된 텍스처 좌표
    float3 normal : NORMAL; // 월드 공간에서의 법선 벡터
    float3 viewDirection : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (정점 셰이더)
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    
    // 행렬 연산을 위해 w 성분을 1.0으로 설정
    input.position.w = 1.0f;

    // 정점의 위치를 월드 -> 뷰 -> 투영 공간 순으로 변환
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 텍스처 좌표를 픽셀 셰이더로 전달
    output.tex = input.tex;

    // 법선 벡터를 월드 공간으로 변환 (회전/스케일 반영)
    // 위치 이동을 배제하기 위해 float3x3으로 캐스팅하여 연산함
    output.normal = mul(input.normal, (float3x3) worldMatrix);

    // 변환된 법선 벡터를 정규화(단위 벡터화)
    output.normal = normalize(output.normal);
    
    // 시선 방향(Viewing Direction)은 여기 정점 셰이더에서 계산됩니다.
    // 먼저 정점의 월드 공간 좌표(World Position)를 계산한 뒤, 
    // 이를 카메라 위치(Camera Position)에서 빼서 우리가 장면을 어디서 바라보고 있는지 결정합니다.
    // 최종 값은 정규화(Normalize)되어 픽셀 셰이더로 전달됩니다.
    
    // 정점의 월드 좌표계를 계산합니다.
    worldPosition = mul(input.position, worldMatrix);
    // 카메라의 위치와 월드 좌표계상의 정점 위치를 기준으로 시야 방향을 결정합니다.
    // 시야 방향 벡터를 정규화합니다.
    output.viewDirection = normalize(cameraPosition - worldPosition.xyz);
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader (픽셀 셰이더)
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture : register(t0); // 텍스처 리소스
SamplerState SampleType : register(s0); // 샘플러 상태

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection;
    float4 specular;
    
    // 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처의 픽셀 색상을 샘플링합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    // 모든 픽셀의 기본 출력 색상을 주변광 값으로 설정합니다.
    color = ambientColor;
    // 반사광 색상을 초기화합니다.
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    // 계산을 위해 빛의 방향을 반전시킵니다.
    lightDir = -lightDirection;
    // 이 픽셀에 도달하는 빛의 양을 계산합니다.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    if (lightIntensity > 0.0f)
    {
        // 확산광 색상과 광량에 따라 최종 확산광 색상을 결정합니다.
        color += (diffuseColor * lightIntensity);
        // 주변광과 확산광의 색상을 포함시킵니다.
        color = saturate(color);
        // 광도, 법선 벡터 및 광 방향을 기반으로 반사 벡터를 계산합니다.
        reflection = normalize(2.0f * lightIntensity * input.normal - lightDir);
        // 반사 벡터, 시야 방향 및 반사광 강도를 기반으로 반사광의 양을 결정합니다.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
    }
    
    // 텍스처 픽셀과 최종 확산 색상을 곱하여 최종 픽셀 색상 결과를 얻습니다.
    color = color * textureColor;

    // 반사광 성분을 출력 색상의 마지막에 추가합니다.
    color = saturate(color + specular);
    
    return color;
}