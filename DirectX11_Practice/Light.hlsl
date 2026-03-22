/////////////
// DEFINES //
/////////////

/////////////
// GLOBALS //
/////////////

// Matrix buffer: transforms vertices into 3D space
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix; // World transform matrix
    matrix viewMatrix; // View transform matrix
    matrix projectionMatrix; // Projection transform matrix
};

// HLSL allows arrays using a defined element count.
// This array stores the positions of four point lights.
cbuffer LightPositionBuffer : register(b1)
{
    float4 lightPosition[4];
};

// This array stores the diffuse colors of four point lights.
cbuffer LightColorBuffer : register(b2)
{
    float4 diffuseColor[4];
};

//////////////
// TYPEDEFS //
//////////////

// Vertex shader input structure
struct VertexInputType
{
    float4 position : POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // Normal vector for lighting
};

// Pixel shader input structure
// This is the output from the vertex shader.
struct PixelInputType
{
    float4 position : SV_POSITION; // Projected screen position
    float2 tex : TEXCOORD0; // Interpolated texture coordinates
    float3 normal : NORMAL; // Normal in world space
    float3 lightPos[4] : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    int i;

    // Set w to 1.0 for matrix multiplication
    input.position.w = 1.0f;

    // Transform position from world to view to projection space
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Pass texture coordinates to the pixel shader
    output.tex = input.tex;

    // Transform the normal into world space
    // Cast to float3x3 to ignore translation
    output.normal = mul(input.normal, (float3x3) worldMatrix);

    // Normalize the transformed normal
    output.normal = normalize(output.normal);

    // Compute the world position of the vertex
    worldPosition = mul(input.position, worldMatrix);

    // Compute the direction from the vertex to each light
    // and normalize each vector before passing it to the pixel shader
    for (i = 0; i < 4; i++)
    {
        output.lightPos[i] = lightPosition[i].xyz - worldPosition.xyz;
        output.lightPos[i] = normalize(output.lightPos[i]);
    }

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture : register(t0); // Texture resource
SamplerState SampleType : register(s0); // Sampler state

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float lightIntensity[4];
    float4 colorArray[4];
    float4 colorSum;
    float4 color;
    int i;

    // Sample the texture color at this UV coordinate
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // Compute the light intensity and diffuse contribution
    // for each point light
    for (i = 0; i < 4; i++)
    {
        lightIntensity[i] = saturate(dot(input.normal, input.lightPos[i]));
        colorArray[i] = diffuseColor[i] * lightIntensity[i];
    }

    // Sum all light contributions
    colorSum = float4(0.0f, 0.0f, 0.0f, 1.0f);

    for (i = 0; i < 4; i++)
    {
        colorSum.r += colorArray[i].r;
        colorSum.g += colorArray[i].g;
        colorSum.b += colorArray[i].b;
    }

    // Multiply the texture color by the summed lighting
    color = saturate(colorSum) * textureColor;

    return color;
}