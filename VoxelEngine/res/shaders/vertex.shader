#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;

uniform mat4 u_MVP;

out vec2 v_TexCoord;
out float v_VertexAO;

void main()
{
    gl_Position = u_MVP * vec4(position, 1.0);
    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;
}
