#version 330 core

uniform sampler2D u_DepthTex;
in vec2 TexCoords;
out vec4 FragColor;

void main() {
    float depth = texture(u_DepthTex, TexCoords).r;
    FragColor = vec4(vec3(depth), 1.0); // visualize depth
}