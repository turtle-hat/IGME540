#ifndef __GGP_SHADER_LIGHTING__
#define __GGP_SHADER_LIGHTING__

#include "ShaderStructs.hlsli"

#define LIGHT_COUNT	8

// Calculates Lambert diffuse shading for a pixel
float DiffuseLambert(float3 lightDirectionOut, float3 surfaceNormal)
{
    // Get dot product of normal and light direction, scale by intensity and the color of the light and surface 
    return saturate(dot(surfaceNormal, lightDirectionOut));
}

// Calculates Phong specular shading for a pixel 
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

// Calculates the strength of a point light for a pixel. Returns a value that attenuates the light when multiplied
float Attenuate(Light light, float3 worldPosition)
{
    float attenuationDistance = distance(light.Position, worldPosition);
    float attenuation = saturate(1.0f - (pow(attenuationDistance, 2) / pow(light.Range, 2)));
    return pow(attenuation, 2);
}

// Calculates the strength of a spot light for a pixel. Returns a value that attenuates the light when multiplied
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

// Calculates Lambert-Phong lighting result of a single light treated as a directional light
float3 LightDirectionalLambertPhong(Light light, float3 surfaceNormal, float3 surfaceColor, float surfaceRoughness, float surfaceSpecular, float3 surfaceWorldPos, float3 cameraPosition)
{
    float3 lightDirectionIn = normalize(light.Direction);
    
    float diffuse = DiffuseLambert(-lightDirectionIn, surfaceNormal);
    float specular = surfaceSpecular * SpecularPhong(lightDirectionIn, surfaceNormal, surfaceRoughness, surfaceWorldPos, cameraPosition);

    return light.Color * light.Intensity * (surfaceColor * diffuse + specular);
}

// Calculates Lambert-Phong lighting result of a single light treated as a point light
float3 LightPointLambertPhong(Light light, float3 surfaceNormal, float3 surfaceColor, float surfaceRoughness, float surfaceSpecular, float3 surfaceWorldPos, float3 cameraPosition)
{
    float3 lightDirectionIn = normalize(surfaceWorldPos - light.Position);
    
    float diffuse = DiffuseLambert(-lightDirectionIn, surfaceNormal);
    float specular = surfaceSpecular * SpecularPhong(lightDirectionIn, surfaceNormal, surfaceRoughness, surfaceWorldPos, cameraPosition);

    return light.Color * light.Intensity * (surfaceColor * diffuse + specular) * Attenuate(light, surfaceWorldPos);
}

// Calculates Lambert-Phong lighting result of a single light treated as a spot light
float3 LightSpotLambertPhong(Light _light, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float3 lightDirectionIn = normalize(_surfaceWorldPos - _light.Position);
    
    return LightPointLambertPhong(_light, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition) * SpotTerm(_light, lightDirectionIn);
}

// Calculates Lambert-Phong lighting result of an array of lights
float3 CalculateLightingLambertPhong(Light _lights[LIGHT_COUNT], float3 _lightAmbient, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    // Start accumulator with ambient light value
    float3 lightsFinal = _lightAmbient;

	// For each light in the scene
    for (uint i = 0; i < LIGHT_COUNT; i++)
    {
		// If it's active
        if (_lights[i].Active)
        {
			// Run a different lighting equation on it depending on the type of light
            switch (_lights[i].Type)
            {
                case LIGHT_TYPE_DIRECTIONAL:
                    lightsFinal += LightDirectionalLambertPhong(_lights[i], _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition);
                    break;
                case LIGHT_TYPE_POINT:
                    lightsFinal += LightPointLambertPhong(_lights[i], _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition);
                    break;
                case LIGHT_TYPE_SPOT:
                    lightsFinal += LightSpotLambertPhong(_lights[i], _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition);
                    break;
                default:
                    break;
            }
        }
    }
    
    return lightsFinal;
}

#endif
