#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out float gMetallic;
layout (location = 4) out float gRoughness;
layout (location = 5) out float gAO;

in vec3 v_FragPos;
in vec2 v_TexCoord;
in float v_VertexAO;

uniform sampler2D u_Texture;
uniform sampler2D u_NormalMap;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    if (texColor.a < 0.1)
        discard;

    // Output Position
    gPosition = v_FragPos;

    // Reconstruct Geometric Normal (Flat Shading)
    vec3 dx = dFdx(v_FragPos);
    vec3 dy = dFdy(v_FragPos);
    vec3 geomNormal = normalize(cross(dx, dy));

    // Calculate TBN
    mat3 TBN = cotangent_frame(geomNormal, v_FragPos, v_TexCoord);

    // Sample Normal Map
    vec3 mapNormal = texture(u_NormalMap, v_TexCoord).rgb;
    mapNormal = normalize(mapNormal * 2.0 - 1.0);

    // Transform to World Space
    gNormal = normalize(TBN * mapNormal);

    // Albedo
    gAlbedo = vec4(texColor.rgb, 1.0);

    // PBR Properties (Hardcoded for blocks for now)
    gMetallic = 0.0;
    gRoughness = 0.9;
    
    // Ambient Occlusion (from vertex)
    gAO = 1.0 - (v_VertexAO * 0.6); // Tune intensity
}
