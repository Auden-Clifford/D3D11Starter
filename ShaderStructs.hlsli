#ifndef __SHADER_STRUCTS__ // Each .hlsli file needs a unique identifier! 
#define __SHADER_STRUCTS__

#define LIGHT_TYPE_DIRECTIONAL  0
#define LIGHT_TYPE_POINT        1
#define LIGHT_TYPE_SPOT         2

#define MAX_SPECULAR_EXPONENT 256.0f



// struct that holds data passed from the vertex shader to the pixel shader
struct VertexToPixel
{
    float4 screenPosition   : SV_POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 worldPosition    : POSITION;
};

// Struct representing a single vertex worth of data used by the vertex shader
struct VertexShaderInput
{
    float3 localPosition    : POSITION; // XYZ position
    float2 uv               : TEXCOORD; // texture coordinates
    float3 normal           : NORMAL; // normal vector
    float3 tangent          : TANGENT;
};

struct Light
{
    int Type;               // Which kind of light? 0, 1 or 2 (see above)
    float3 Direction;       // Directional and Spot lights need a direction
    float Range;            // Point and Spot lights have a max range for attenuation
    float3 Position;        // Point and Spot lights have a position in space
    float Intensity;        // All lights need an intensity
    float3 Color;           // All lights need a color
    float SpotInnerAngle;   // Inner cone angle (in radians) – Inside this, full light!
    float SpotOuterAngle;   // Outer cone angle (radians) – Outside this, no light!
    float2 Padding;         // Purposefully padding to hit the 16-byte boundary
};

// ALL of your code pieces (structs, functions, etc.) go here!
#endif