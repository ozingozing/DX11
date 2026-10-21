////////////////////////////////////////////////////////////////////////////////
// 파일명: clipplane.hlsl
// 설명: 단일 클립 평면(Clip Plane)을 처리하도록 수정된 텍스처 셰이더입니다.
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

// 클립 평면 버퍼: 외부 객체에서 클립 평면의 위치와 잘려나갈(clipping) 방향을 설정할 수 있는 4개의 float 벡터입니다.
// 앞선 3개의 float은 클립 평면의 법선 벡터(위치)를 지정하고, 네 번째 float은 평면의 위치(오프셋)를 나타냅니다.
// 예를 들어 (0.0f, 1.0f, 0.0f, -5.0f)인 경우 Y축 아래의 모든 것을 클리핑하며, 높이 5.0에서 이 클리핑이 시작됩니다.
// 즉, Y축 기준으로 5.0 미만에 있는 씬의 모든 픽셀은 절대 그려지지 않습니다.
// 이 튜토리얼에서는 (0.0f, -1.0f, 0.0f, 0.0f) 값을 사용하여 Y축 0.0 초과인 부분을 컬링(제외)합니다.
cbuffer ClipPlaneBuffer
{
    float4 clipPlane;
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
    // 픽셀 셰이더의 입력으로 필요한 클립 평면 시멘틱(semantic)입니다.
    float clip : SV_ClipDistance0;
};


////////////////////////////////////////////////////////////////////////////////
// 버텍스 셰이더 (Vertex Shader)
////////////////////////////////////////////////////////////////////////////////
PixelInputType ClipPlaneVertexShader(VertexInputType input)
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

    // 픽셀 셰이더 입력을 위한 클립 평면을 설정하기 위해, 월드 공간에서의 입력 버텍스와 클립 평면 간의 내적(dot product)을 수행합니다.
    // 이제 클립 평면이 활성화되며, 픽셀 셰이더는 이 클립 평면에 의해 컬링되는 어떠한 것도 그리지 않게 됩니다.
    // 클리핑 평면 설정
    output.clip = dot(mul(input.position, worldMatrix), clipPlane);

    return output;
}


////////////////////////////////////////////////////////////////////////////////
// 픽셀 셰이더 (Pixel Shader)
////////////////////////////////////////////////////////////////////////////////
float4 ClipPlanePixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

    // 해당 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처의 픽셀 색상을 샘플링합니다.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}