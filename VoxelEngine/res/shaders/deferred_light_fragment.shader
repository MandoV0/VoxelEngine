#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetallic;
uniform sampler2D gRoughness;
uniform sampler2D gAO;
uniform sampler2D shadowMap;
uniform sampler2D ssgiMap;

struct DirLight {
    vec3 direction;
    vec3 color;
};
uniform DirLight dirLight;
uniform vec3 viewPos;
uniform mat4 u_LightSpaceMatrix;

// FOG Params
uniform vec3  u_FogColor;
uniform float u_FogDensity;
uniform float u_FogHeight;
uniform float u_FogFalloff;
uniform int   u_FogMode;

const float PI = 3.14159265359;

// ---------------------- SHADOW ----------------------
float ShadowCalculation(vec3 fragPosWorld)
{
    vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(fragPosWorld, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - 0.001 > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

// ---------------------- PBR ----------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---------------------- ACES Tone Mapping ----------------------
vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
}

// ---------------------- MAIN ----------------------
void main() {
    // Retrieve data from G-Buffer
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    float Metallic = texture(gMetallic, TexCoords).r;
    float Roughness = texture(gRoughness, TexCoords).r;
    float AO = texture(gAO, TexCoords).r;

    if (length(Normal) == 0.0) {
        discard; 
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Albedo, Metallic);

    // --- Directional Light ---
    vec3 L = normalize(-dirLight.direction);
    vec3 H = normalize(V + L);

    vec3 radiance = dirLight.color;

    float NDF = DistributionGGX(N, H, Roughness);
    float G   = GeometrySmith(N, V, L, Roughness);
    vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - Metallic);

    float NdotL = max(dot(N, L), 0.0);

    float shadow = ShadowCalculation(WorldPos); 
    vec3 Lo = (kD * Albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);

    // --- Ambient ---
    vec3 ssgi = texture(ssgiMap, TexCoords).rgb;
    vec3 ambient = (vec3(0.03) + ssgi) * Albedo * AO;
    vec3 color = ambient + Lo;

    // --- Fog ---
    float dist = length(viewPos - WorldPos);
    float fogFactor = 0.0;
    if (u_FogMode == 0) {
        fogFactor = 1.0 - exp(-dist * u_FogDensity);
    } else {
        float heightDelta = max(WorldPos.y - u_FogHeight, 0.0);
        float heightFactor = exp(-heightDelta * u_FogFalloff);
        fogFactor = 1.0 - exp(-dist * u_FogDensity * heightFactor);
    }
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    color = mix(color, u_FogColor, fogFactor);

    // --- ACES Tone Mapping + sRGB Gamma ---
    color = ACESFilm(color);
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
