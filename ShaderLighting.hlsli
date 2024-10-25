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

float Attenuate(Light light, float3 worldPosition)
{
    float attenuationDistance = distance(light.Position, worldPosition);
    float attenuation = saturate(1.0f - (pow(attenuationDistance, 2) / pow(light.Range, 2)));
    return pow(attenuation, 2);
}

float SpotTerm(Light light, float3 lightDirectionIn)
{
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    
    return saturate(
        (cosOuter - 
            saturate(dot(lightDirectionIn, light.Direction)) // Angle from pixel to light direction
        ) /
        (cosOuter - cosInner) // Falloff range
    );

}

float3 LightDirectional(Light light, float3 surfaceNormal, float3 surfaceColor, float surfaceRoughness, float3 surfaceWorldPos, float3 cameraPosition)
{
    float3 lightDirectionIn = normalize(light.Direction);
    
    float diffuse = DiffuseLambert(-lightDirectionIn, surfaceNormal);
    float specular = SpecularPhong(lightDirectionIn, surfaceNormal, surfaceRoughness, surfaceWorldPos, cameraPosition);

    return light.Color * light.Intensity * (surfaceColor * diffuse + specular);
}

float3 LightPoint(Light light, float3 surfaceNormal, float3 surfaceColor, float surfaceRoughness, float3 surfaceWorldPos, float3 cameraPosition)
{
    float3 lightDirectionIn = normalize(surfaceWorldPos - light.Position);
    
    float diffuse = DiffuseLambert(-lightDirectionIn, surfaceNormal);
    float specular = SpecularPhong(lightDirectionIn, surfaceNormal, surfaceRoughness, surfaceWorldPos, cameraPosition);

    return light.Color * light.Intensity * (surfaceColor * diffuse + specular) * Attenuate(light, surfaceWorldPos);
}

float3 LightSpot(Light light, float3 surfaceNormal, float3 surfaceColor, float surfaceRoughness, float3 surfaceWorldPos, float3 cameraPosition)
{
    float3 lightDirectionIn = normalize(surfaceWorldPos - light.Position);
    
    return LightPoint(light, surfaceNormal, surfaceColor, surfaceRoughness, surfaceWorldPos, cameraPosition) * SpotTerm(light, lightDirectionIn);
}

#endif
