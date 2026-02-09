#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform mat4 u_View;
uniform mat4 u_Proj;

const int SAMPLES = 16;
const float RADIUS = 0.5;
const float BIAS = 0.025;

// Pseudo-random number generator
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    // Get World Space data
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;
    vec3 WorldNormal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Optimization: Skip if no geometry (assuming 0,0,0 is invalid/sky or checking alpha if available)
    // Using length(WorldNormal) is a good proxy if normals are always normalized on valid geo.
    if (length(WorldNormal) < 0.1) {
        FragColor = vec4(0.0);
        return;
    }

    // Convert to View Space
    vec3 ViewPos = vec3(u_View * vec4(WorldPos, 1.0));
    vec3 ViewNormal = mat3(u_View) * WorldNormal;

    vec3 randomVec = normalize(vec3(rand(TexCoords), rand(TexCoords + 0.1), rand(TexCoords + 0.2)) * 2.0 - 1.0);

    vec3 Tangent = normalize(randomVec - ViewNormal * dot(randomVec, ViewNormal));
    vec3 Bitangent = cross(ViewNormal, Tangent);
    mat3 TBN = mat3(Tangent, Bitangent, ViewNormal);

    vec3 indirectLighting = vec3(0.0);
    float occlusion = 0.0;

    for(int i = 0; i < SAMPLES; ++i)
    {
        // Simple Hemisphere sample (should use a better kernel, but random is okay for noise)
        vec3 samplePos = vec3(
            rand(TexCoords + float(i) * 0.1) * 2.0 - 1.0,
            rand(TexCoords + float(i) * 0.2) * 2.0 - 1.0,
            rand(TexCoords + float(i) * 0.3)
        );
        samplePos = normalize(samplePos);
        samplePos = samplePos * rand(TexCoords + float(i) * 0.4); // Random scale
        float scale = float(i) / float(SAMPLES);
        scale = mix(0.1f, 1.0f, scale * scale);
        samplePos *= scale;

        vec3 sampleDir = TBN * samplePos;
        vec3 samplePoint = ViewPos + sampleDir * RADIUS; 
        
        // Project to Screen Space
        vec4 offset = vec4(samplePoint, 1.0);
        offset = u_Proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        // Check depth
        vec3 sampleWorldPos = texture(gPosition, offset.xy).rgb;
        if(length(sampleWorldPos) == 0.0) continue; // Sampled sky

        vec3 sampleViewPos = vec3(u_View * vec4(sampleWorldPos, 1.0));

        // Range check & Accumulate
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(ViewPos.z - sampleViewPos.z));
        
        if (sampleViewPos.z >= samplePoint.z + BIAS) // Sample is behind geometry (in view space z is negative, wait. OpenGL view space looks down -Z)
        {
             // Standard OpenGL: Camera at 0, looking -Z. 
             // If point A is -5 and point B is -10. A is closer. A > B.
             // If samplePoint is inside the geometry, its Z (-5.1) should be less than geometry Z (-5.0).
             // Let's re-verify depth test direction.
             // Usually: if (geometryDepth > sampleDepth) -> Occlusion
             // Here ViewPos.z is negative.
             // If sampleViewPos.z (-4.9) > samplePoint.z (-5.1) -> Hit surface is closer to camera -> Occlusion.
             
             vec3 bounceColor = texture(gAlbedo, offset.xy).rgb;
             indirectLighting += bounceColor * 0.5 * rangeCheck; // 0.5 factor for intensity
             occlusion += rangeCheck;
        }
    }
    
    // Normalize
    indirectLighting = (indirectLighting / float(SAMPLES));
    // Amplify
    FragColor = vec4(indirectLighting * 5.0, 1.0);
}
