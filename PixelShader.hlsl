#include "ShaderStructs.hlsli"
#include "LightingEquations.hlsli"

// texture and sampler
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
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
    input.tangent = normalize(input.tangent);
    
    float2 uvPosition = input.uv * uvScale + uvOffset;
    
    // sample the textures 
    float3 albedoColor = pow(Albedo.Sample(BasicSampler, uvPosition), 2.2f);
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    // unpack the normal from the normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, uvPosition).rgb * 2 - 1;
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
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // calculate the camera view vector
    float3 view = normalize(cameraPos - input.worldPosition);
    
    // ambient light calculations
    //float3 ambientTerm = ambient * albedoColor;
    
    // calculate light from all lights
    float3 result = float3(0, 0, 0);
    for (int i = 0; i < 5; i++)
    {
        if (lights[i].Type == 0)
        {
            result += CalculateDirectionalLight(lights[i], input.normal, view, albedoColor, specularColor, roughness, metalness);
        }
        else if (lights[i].Type == 1)
        {
            result += CalculatePointLight(lights[i], input.normal, view, albedoColor, specularColor, roughness, metalness, input.worldPosition);
        }
        else
        {
            result += CalculateSpotLight(lights[i], input.normal, view, albedoColor, specularColor, roughness, metalness, input.worldPosition);
        }
    
    }
    
	// return texture color multiplied by tint
    return float4(pow(result, 1.0f / 2.2f), 1);
}