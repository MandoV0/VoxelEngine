#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec3 v_FragPos;
out float v_FogDepth;
out vec2 v_TexCoord;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    vec4 viewPos = u_View * worldPos;

    v_FragPos = vec3(worldPos);
    v_FogDepth = -viewPos.z; // distance along camera forward
    v_TexCoord = a_TexCoord;

    gl_Position = u_Proj * viewPos;
}
