#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;
layout(location = 3) in float lightLevel;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec2 v_TexCoord;
out float v_VertexAO;
out float v_LightLevel;
out float v_FogDepth;
out float v_WorldY; // height for height fog

void main()
{
    vec4 worldPos = u_Model * vec4(position, 1.0);
    vec4 viewPos = u_View * worldPos;

    gl_Position = u_Proj * viewPos;

    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;
    v_LightLevel = lightLevel;
    v_FogDepth = -viewPos.z; // camera distance
    v_WorldY = worldPos.y;   // world space height
}
