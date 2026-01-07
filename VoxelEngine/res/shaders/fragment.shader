#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexAO;
in float v_LightLevel;

uniform sampler2D u_Texture;

void main()
{
	color = texture( u_Texture, v_TexCoord );
	color *= (1 - v_VertexAO * 0.3);
	color *= v_LightLevel;
}