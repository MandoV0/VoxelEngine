#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;
layout(location = 3) in float lightLevel;

uniform mat4 u_MVP;
uniform float u_Time;

out vec2 v_TexCoord;
out float v_VertexAO;

void main()
{
    vec3 pos = position;

    float wave = sin(u_Time * 1.5 + pos.x * 0.8 + pos.z * 0.8) * 0.08;
    pos.y += wave;

    gl_Position = u_MVP * vec4(pos, 1.0);

    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;
}
