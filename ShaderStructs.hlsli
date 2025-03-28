#ifndef __SHADER_STRUCTS__ // Each .hlsli file needs a unique identifier! 
#define __SHADER_STRUCTS__
// ALL of your code pieces (structs, functions, etc.) go here!
#endif

// struct that holds data passed from the vertex shader to the pixel shader
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
	//float4 color			: COLOR;
};

// Struct representing a single vertex worth of data used by the vertex shader
struct VertexShaderInput
{
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD; // texture coordinates
    float3 normal : NORMAL; // normal vector
};