#include "ShaderStructs.hlsli"
#include "LightingEquations.hlsli"

// texture and sampler
Texture2D SurfaceTexture    : register(t0); 
Texture2D NormalMap         : register(t1);
SamplerState BasicSampler   : register(s0);

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
    input.tangent = normalize(input.tangent);
    
    // unpack the normal from the normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv * uvScale + uvOffset).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal); // Don�t forget to normalize!
    
    // Feel free to adjust/simplify this code to fit with your existing shader(s)
    // Simplifications include not re-normalizing the same vector more than once!
    float3 N = input.normal; 
    float3 T = input.tangent;
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    // transform the normal by the the value from the normal map
    input.normal = mul(unpackedNormal, TBN);
    
	// sample the texture 
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv * uvScale + uvOffset); 
    
    // calculate the camera view vector
    float3 view = normalize(cameraPos - input.worldPosition);
    
    // ambient light calculations
    float3 ambientTerm = ambient * surfaceColor.rgb;
    
    // calculate light from all lights
    float3 result = float3(0, 0, 0);
    for (int i = 0; i < 5; i++)
    {
        if (lights[i].Type == 0)
        {
            result += CalculateDirectionalLight(lights[i], input.normal, surfaceColor, roughness, cameraPos, view);
        }
        else if (lights[i].Type == 1)
        {
            result += CalculatePointLight(lights[i], input.normal, surfaceColor, roughness, cameraPos, view, input.worldPosition);
        }
        else
        {
            result += CalculateSpotLight(lights[i], input.normal, surfaceColor, roughness, cameraPos, view, input.worldPosition);
        }
    
    }
    
	// return texture color multiplied by tint
    return float4(result + ambientTerm, 1);
}