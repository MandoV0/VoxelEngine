#version 330 core

layout(location = 0) out vec4 FragColor;



in vec3 v_FragPos;

in vec2 v_TexCoord;

in float v_VertexAO;

in vec3 v_Normal;



uniform sampler2D u_Texture;

uniform sampler2D shadowMap;

uniform vec3 viewPos;

uniform mat4 u_LightSpaceMatrix;



struct DirLight {

    vec3 direction;

    vec3 color;

};

uniform DirLight dirLight;



// FOG Params

uniform vec3  u_FogColor;

uniform float u_FogDensity;

uniform float u_FogHeight;

uniform float u_FogFalloff;

uniform int   u_FogMode;



const float PI = 3.14159265359;



float ShadowCalculation(vec3 fragPosWorld)

{

    vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(fragPosWorld, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;

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



void main()

{

    vec4 texColor = texture(u_Texture, v_TexCoord);

    

    // Ensure water is visible. If texture is fully opaque, use 0.7 alpha.

    // If texture is already transparent, use its alpha.

    float alpha = texColor.a;

    if (alpha > 0.9) alpha = 0.7; 



    vec3 albedo = texColor.rgb;



    vec3 N = normalize(v_Normal);

    vec3 V = normalize(viewPos - v_FragPos);

    vec3 L = normalize(-dirLight.direction);

    vec3 H = normalize(V + L);

    

    // Simple Blinn-Phong/PBR approximation for water surface

    float NdotL = max(dot(N, L), 0.0);

    

    float shadow = ShadowCalculation(v_FragPos);

    

    vec3 diffuse = albedo * NdotL * dirLight.color * (1.0 - shadow);

    

    // Specular highlight

    float spec = pow(max(dot(N, H), 0.0), 128.0); // Sharper highlight

    vec3 specular = vec3(0.8) * spec * dirLight.color * (1.0 - shadow);



    vec3 ambient = vec3(0.1) * albedo * (1.0 - v_VertexAO * 0.5);

    vec3 finalColor = ambient + diffuse + specular;



    // --- Fog ---

    float dist = length(viewPos - v_FragPos);

    float fogFactor = 0.0;

    if (u_FogMode == 0) {

        fogFactor = 1.0 - exp(-dist * u_FogDensity);

    } else {

        float heightDelta = max(v_FragPos.y - u_FogHeight, 0.0);

        float heightFactor = exp(-heightDelta * u_FogFalloff);

        fogFactor = 1.0 - exp(-dist * u_FogDensity * heightFactor);

    }

    fogFactor = clamp(fogFactor, 0.0, 1.0);

    

    finalColor = mix(finalColor, u_FogColor, fogFactor);



    // Tone mapping & Gamma

    finalColor = finalColor / (finalColor + vec3(1.0));

    finalColor = pow(finalColor, vec3(1.0/2.2));



    FragColor = vec4(finalColor, alpha);

}
