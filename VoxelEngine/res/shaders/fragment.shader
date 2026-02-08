#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexAO;
in float v_LightLevel;
in float v_FogDepth;
in float v_WorldY;

uniform sampler2D u_Texture;

// FOG
uniform vec3  u_FogColor;
uniform float u_FogDensity;
uniform float u_FogHeight;
uniform float u_FogFalloff;
uniform int   u_FogMode;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    if (texColor.a < 0.1)
        discard;

    vec3 finalColor = texColor.rgb;
    finalColor *= (1.0 - v_VertexAO * 0.3);
    finalColor *= v_LightLevel;

    float fogFactor = 0.0;

    if (u_FogMode == 0)
    {
        // Exponential fog
        fogFactor = 1.0 - exp(-v_FogDepth * u_FogDensity);
    }
    else
    {
        // Exponential height fog
        float heightDelta  = max(v_WorldY - u_FogHeight, 0.0);
        float heightFactor = exp(-heightDelta * u_FogFalloff);
        fogFactor = 1.0 - exp(-v_FogDepth * u_FogDensity * heightFactor);
    }

    fogFactor = clamp(fogFactor, 0.0, 1.0);

    finalColor = mix(u_FogColor, finalColor, 1.0 - fogFactor);

    color = vec4(finalColor, texColor.a);
}
