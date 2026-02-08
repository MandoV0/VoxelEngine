#version 330 core

in vec3 v_FragPos;
in float v_FogDepth;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform vec3 u_FogColor;
uniform float u_FogStart;
uniform float u_FogEnd;

// For textures
uniform sampler2D u_Texture;

void main()
{
    vec3 objectColor = texture(u_Texture, v_TexCoord).rgb;

    // Linear Fog
    float fogFactor = clamp((u_FogEnd - v_FogDepth) / (u_FogEnd - u_FogStart), 0.0, 1.0);

    vec3 finalColor = mix(u_FogColor, objectColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}