#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float vertexAO;
layout(location = 3) in float lightLevel;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec3 v_FragPos;
out vec2 v_TexCoord;
out float v_VertexAO;

void main()
{
    vec4 worldPos = u_Model * vec4(position, 1.0);
    v_FragPos = worldPos.xyz;
    
    gl_Position = u_Proj * u_View * worldPos;

    v_TexCoord = texCoord;
    v_VertexAO = vertexAO;
}
