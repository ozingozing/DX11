////////////////////////////////////////////////////////////////////////////////
// 설명: 텍스처 매핑을 지원하는 정점 및 픽셀 셰이더입니다.
////////////////////////////////////////////////////////////////////////////////

/////////////
// 전역 변수 //
/////////////

// 행렬 버퍼: 월드, 뷰, 투영 행렬을 저장하여 정점의 위치를 변환하는 데 사용합니다.
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 텍스처 리소스: 모델에 렌더링할 실제 이미지 데이터입니다.
Texture2D shaderTexture : register(t0);

// 샘플러 상태: 텍스처가 폴리곤 면에 그려질 때 픽셀을 어떻게 추출(샘플링)할지 결정합니다.
// 예를 들어 물체가 매우 멀리 있어 작게 보일 때 텍스처가 깨지지 않도록 픽셀을 조합하는 방식을 정의합니다.
SamplerState SampleType : register(s0);


//////////////
// 타입 정의 //
//////////////

// 정점 셰이더 입력 구조체
struct VertexInputType
{
    float4 position : POSITION;
    // 이전의 색상 대신 텍스처 좌표(UV)를 사용합니다. 
    // U, V 두 개의 부동 소수점을 가지므로 float2 타입을 사용하며 시맨틱은 TEXCOORD0입니다.
    float2 tex : TEXCOORD0;
};

// 픽셀 셰이더 입력 구조체 (정점 셰이더 출력)
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// 정점 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    

    // 행렬 계산을 위해 위치 벡터의 w 성분을 1.0으로 설정합니다.
    input.position.w = 1.0f;

    // 정점 위치를 월드, 뷰, 투영 행렬과 곱하여 화면상의 좌표로 계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 입력받은 텍스처 좌표를 복사하여 픽셀 셰이더로 전달합니다.
    output.tex = input.tex;

    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

    // HLSL의 Sample 함수를 사용하여 텍스처의 색상을 결정합니다.
    // 설정된 샘플러 상태(SampleType)와 정점 셰이더에서 넘어온 UV 좌표(input.tex)를 사용하여
    // 폴리곤 면의 해당 위치에 표시될 최종 픽셀 값을 추출합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}