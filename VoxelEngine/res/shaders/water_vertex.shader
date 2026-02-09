#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;
layout(location = 3) in float lightLevel;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform float u_Time;

out vec3 v_FragPos;
out vec2 v_TexCoord;
out float v_VertexAO;
out vec3 v_Normal;

void main()
{
    vec3 pos = position;

    // Wave parameters: Amplitude=0.08, Frequency=1.5, Kx=0.8, Kz=0.8
    float arg = u_Time * 1.5 + pos.x * 0.8 + pos.z * 0.8;
    float wave = sin(arg) * 0.08;
    pos.y += wave;

    vec4 worldPos = u_Model * vec4(pos, 1.0);
    v_FragPos = worldPos.xyz;
    gl_Position = u_Proj * u_View * worldPos;

    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;

    // Analytical Normal
    float derivative = cos(arg) * 0.08; // derivative of sin is cos
    // Partial derivatives
    float dx = derivative * 0.8;
    float dz = derivative * 0.8;
    
    // Normal = normalize(-dx, 1, -dz)
    v_Normal = normalize(vec3(-dx, 1.0, -dz));
}
