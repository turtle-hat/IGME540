#ifndef __GGP_SHADER_LIGHTING__
#define __GGP_SHADER_LIGHTING__

#include "ShaderStructs.hlsli"

float DiffuseLambert(float3 lightDirectionOut, float3 surfaceNormal)
{
    // Get dot product of normal and light direction, scale by intensity and the color of the light and surface 
    return saturate(dot(surfaceNormal, lightDirectionOut));
}

float SpecularPhong(float3 lightDirectionIn, float3 surfaceNormal, float surfaceRoughness, float3 surfaceWorldPos, float3 cameraPosition)
{
    return pow(
        saturate(dot(
            reflect(lightDirectionIn, surfaceNormal), // R
            normalize(cameraPosition - surfaceWorldPos) // V
        )),
        (1.0f - min(surfaceRoughness, 0.99f)) * MAX_SPECULAR_EXPONENT // Specular Exponent
    );
}

#endif
