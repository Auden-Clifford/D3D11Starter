#include "ShaderStructs.hlsli"
#include "LightingEquations.hlsli"

// texture and sampler
Texture2D SurfaceTexture : register(t0); 
SamplerState BasicSampler : register(s0);

//constant buffer input
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
    float roughness;
    float3 cameraPos;
    float3 ambient;
    Light lights[5];
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // re-normalize the normals
    input.normal = normalize(input.normal);
    
	// sample the texture 
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv * uvScale + uvOffset); 
    
    // calculate the camera view vector
    float3 view = normalize(cameraPos - input.worldPosition);
    
    // ambient light calculations
    float3 ambientTerm = ambient * surfaceColor;
    
    // calculate light from all lights
    float3 result = float3(0, 0, 0);
    //for (int i = 0; i < 5; i++)
    //{
    //    result += CalculateDirectional(lights[i], input.normal, surfaceColor, roughness, cameraPos, view);
    //}
    
	// return texture color multiplied by tint
    return float4(CalculateDirectional(lights[0], input.normal, surfaceColor, roughness, cameraPos, view) + ambientTerm, 1);
}