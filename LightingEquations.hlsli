#ifndef __LIGHTING_EQUATIONS__ // Each .hlsli file needs a unique identifier! 
#define __LIGHTING_EQUATIONS__

#include "ShaderStructs.hlsli"

// calculates the attenuation of a given light
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// calculates a light's diffuse term
float3 Diffuse(Light light, float3 normal, float3 lightDirection, float4 surfaceColor)
{
    float3 diffuseTerm =
        float4(saturate(dot(normal, -lightDirection)) * // Diffuse intensity, clamped to 0-1
        light.Color * light.Intensity, 1) * // Light’s overall color
        surfaceColor;
    
    return diffuseTerm;
}

// calculates a light's specular term
float3 Specular(Light light, float roughness, float3 lightDirection, float3 normal, float3 view)
{
    float specExponent = (1.0 - roughness) * MAX_SPECULAR_EXPONENT;
    // only do specular if the exponent is larger than 0.05
    float3 specularTerm = float3(0, 0, 0);
    if (specExponent > 0.05f)
    {
        
        float3 reflection = reflect(lightDirection, normal);
        specularTerm = pow(saturate(dot(reflection, view)), specExponent) * light.Color * light.Intensity;
    }
    
    return specularTerm;
}

// calculates the lighting from a directional light
float3 CalculateDirectionalLight(Light light, float3 normal, float4 surfaceColor, float roughness, float3 cameraPos, float3 view)
{
    float3 lightDirection = normalize(light.Direction);
    
    // diffuse light calculations
    float3 diffuseTerm = Diffuse(light, normal, lightDirection, surfaceColor);
    
    // only do specular if the exponent is larger than 0.05
    float3 specularTerm = Specular(light, roughness, lightDirection, normal, view);
    
    return diffuseTerm + specularTerm;
}

// calculates the lighting from a point light
float3 CalculatePointLight(Light light, float3 normal, float4 surfaceColor, float roughness, float3 cameraPos, float3 view, float3 worldPos)
{
    float3 lightDirection = normalize(worldPos - light.Position);
    
    // diffuse light calculations
    float3 diffuseTerm = Diffuse(light, normal, lightDirection, surfaceColor);
    
    // only do specular if the exponent is larger than 0.05
    float3 specularTerm = Specular(light, roughness, lightDirection, normal, view);
    
    return (diffuseTerm + specularTerm) * Attenuate(light, worldPos);
}

float3 CalculateSpotLight(Light light, float3 normal, float4 surfaceColor, float roughness, float3 cameraPos, float3 view, float3 worldPos)
{
    float3 directionToPixel = normalize(worldPos - light.Position);
    
    // Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(directionToPixel, normalize(light.Direction)));
    
    // Get cosines of angles and calculate range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;

    // Linear falloff over the range, clamp 0-1, apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    return CalculatePointLight(light, normal, surfaceColor, roughness, cameraPos, view, worldPos) * spotTerm;
}

// ALL of your code pieces (structs, functions, etc.) go here!
#endif