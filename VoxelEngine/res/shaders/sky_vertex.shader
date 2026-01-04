#version 330 core
layout (location = 0) in vec3 a_Pos;

out vec3 v_texCoord;

uniform mat4 u_Proj;
uniform mat4 u_View;

void main()
{
    v_texCoord = a_Pos;

    mat4 viewNoTranslation = mat4(mat3(u_View));
    vec4 pos = u_Proj * viewNoTranslation * vec4(a_Pos, 1.0);

    gl_Position = pos.xyww;
}
