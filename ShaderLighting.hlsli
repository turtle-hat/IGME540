#ifndef __GGP_SHADER_LIGHTING__
#define __GGP_SHADER_LIGHTING__

float4 DiffuseLambert(float3 surfaceNormal, float3 surfaceColor, float3 directionToLight, float3 lightColor, float lightIntensity)
{
    // Get dot product of normal and light direction, scale by intensity and the color of the light and surface 
    return float4(saturate(dot(surfaceNormal, directionToLight)) * lightColor * lightIntensity * surfaceColor, 1.0f);
}

#endif
