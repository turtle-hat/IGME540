#ifndef __GGP_SHADER_LIGHTING__
#define __GGP_SHADER_LIGHTING__

#include "ShaderStructs.hlsli"

#define LIGHT_COUNT	8

// Calculates Lambert diffuse shading for a pixel
float DiffuseLambert(float3 _lightDirectionOut, float3 _surfaceNormal)
{
    // Get dot product of normal and light direction, scale by intensity and the color of the light and surface 
    return saturate(dot(_surfaceNormal, _lightDirectionOut));
}

// Calculates Phong specular shading for a pixel 
float SpecularPhong(float3 _lightDirectionIn, float3 _surfaceNormal, float _surfaceRoughness, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    return pow(
        saturate(dot(
            reflect(_lightDirectionIn, _surfaceNormal), // R
            normalize(_cameraPosition - _surfaceWorldPos) // V
        )),
        (1.0f - min(_surfaceRoughness, 0.99f)) * MAX_SPECULAR_EXPONENT // Specular Exponent
    );
}

// Calculates the strength of a point light for a pixel. Returns a value that attenuates the light when multiplied
float Attenuate(Light _light, float3 _worldPosition)
{
    float attenuationDistance = distance(_light.Position, _worldPosition);
    float attenuation = saturate(1.0f - (pow(attenuationDistance, 2) / pow(_light.Range, 2)));
    return pow(attenuation, 2);
}

// Calculates the strength of a spot light for a pixel. Returns a value that attenuates the light when multiplied
float SpotTerm(Light _light, float3 _lightDirectionIn)
{
    float cosOuter = cos(_light.SpotOuterAngle);
    float cosInner = cos(_light.SpotInnerAngle);
    
    return saturate(
        (cosOuter - 
            saturate(dot(_lightDirectionIn, _light.Direction)) // Angle from pixel to light direction
        ) /
        (cosOuter - cosInner) // Falloff range
    );

}

// Calculates the Fresnel term for a pixel using Schlick's approximation
float FresnelTermSchlick(float3 _normal, float3 _cameraDirectionOut, float _specularAmount)
{
    return _specularAmount + (1.0f - _specularAmount) * pow(1.0f - saturate(dot(_normal, _cameraDirectionOut)), 5);
}

float LightLambertPhong(Light _light, float3 _lightDirectionIn, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float diffuse = DiffuseLambert(-_lightDirectionIn, _surfaceNormal);
    float specular = _surfaceSpecular * SpecularPhong(_lightDirectionIn, _surfaceNormal, _surfaceRoughness, _surfaceWorldPos, _cameraPosition) * any(diffuse); // Specular is cut if diffuse term is 0
    
    return _light.Color * _light.Intensity * (_surfaceColor * diffuse + specular);
}

// Calculates Lambert-Phong lighting result of a single light treated as a directional light
float3 LightDirectionalLambertPhong(Light _light, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float3 lightDirectionIn = normalize(_light.Direction);
    
    return LightLambertPhong(_light, lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition);
}

// Calculates Lambert-Phong lighting result of a single light treated as a point light
float3 LightPointLambertPhong(Light _light, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float3 lightDirectionIn = normalize(_surfaceWorldPos - _light.Position);
    
    return (
        LightLambertPhong(_light, lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition)
        * Attenuate(_light, _surfaceWorldPos)
    );
}

// Calculates Lambert-Phong lighting result of a single light treated as a spot light
float3 LightSpotLambertPhong(Light _light, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float3 lightDirectionIn = normalize(_surfaceWorldPos - _light.Position);
    
    return (
        LightLambertPhong(_light, lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition)
        * Attenuate(_light, _surfaceWorldPos)
        * SpotTerm(_light, lightDirectionIn)
    );
}

// Calculates Lambert & Phong lighting result of an array of lights
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

// Calculates physically based Lambert & Cook-Torrance lighting result of an array of lights
float3 CalculateLightingLambertCookTorrance(Light _lights[LIGHT_COUNT], float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceMetalness, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    // Start accumulator with ambient light value
    float3 lightsFinal = 0.0f;

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

// Interpolates calculated lit color with reflection using a Fresnel calculation
float3 CalculateReflections(float3 _totalLight, float3 _reflectionColor, float3 _normal, float3 _cameraDirectionOut)
{
    return lerp(
        _totalLight,
        _reflectionColor,
        FresnelTermSchlick(_normal, _cameraDirectionOut, 0.04f)
    );
}

#endif
