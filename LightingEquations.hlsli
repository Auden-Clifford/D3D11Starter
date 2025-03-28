#ifndef __LIGHTING_EQUATIONS__ // Each .hlsli file needs a unique identifier! 
#define __LIGHTING_EQUATIONS__

#include "ShaderStructs.hlsli"




float3 CalculateDirectional(Light light, float3 normal, float4 surfaceColor, float roughness, float3 cameraPos, float3 view)
{
    float3 lightDirection = normalize(light.Direction);
    
    // diffuse light calculations
    float3 diffuseTerm =
        float4(saturate(dot(normal, -lightDirection)) * // Diffuse intensity, clamped to 0-1
        light.Color * light.Intensity, 1) * // Light’s overall color
        surfaceColor;
    
    // specular light calculations
    float specExponent = (1.0 - roughness) * MAX_SPECULAR_EXPONENT;
    // only do specular if the exponent is larger than 0.05
    float3 specularTerm = float3(0, 0, 0);
    if (specExponent > 0.05f)
    {
        
        float3 reflection = reflect(lightDirection, normal);
        specularTerm = pow(saturate(dot(reflection, view)), specExponent) * light.Color * light.Intensity;
    }
    
    return diffuseTerm + specularTerm;
}

// ALL of your code pieces (structs, functions, etc.) go here!
#endif