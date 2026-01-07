#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;
layout(location = 3) in float lightLevel;

uniform mat4 u_MVP;

out vec2 v_TexCoord;
out float v_VertexAO;
out float v_LightLevel;

void main()
{
    gl_Position = u_MVP * vec4(position, 1.0);
    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;
    v_LightLevel = lightLevel;
}
