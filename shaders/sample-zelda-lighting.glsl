@module zelda_lighting
@ctype vec2 sgp_vec2
@ctype float float

@vs vs
layout(location=0) in vec4 coord;
layout(location=1) in vec4 color;
layout(location=0) out vec2 texUV;
layout(location=1) out vec4 iColor;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    texUV = coord.zw;
    iColor = color;
}
@end

@fs fs
layout(binding=0) uniform texture2D iMainTex;    // Main scene texture
layout(binding=1) uniform texture2D iLightTex;   // Light map texture
layout(binding=0) uniform sampler iMainSampler;
layout(binding=1) uniform sampler iLightSampler;
layout(binding=1) uniform fs_uniforms {
    float iAmbientLight;      // Base ambient light level
    float iLightIntensity;    // Light intensity multiplier
    float iTime;              // Time for potential light animation
    float iIsLight;           // Whether this is a light (1.0) or regular texture (0.0)
    vec3 iLightColor;         // Color tint for the light
};

layout(location=0) in vec2 texUV;
layout(location=1) in vec4 iColor;
layout(location=0) out vec4 fragColor;

void main() {
    if (iIsLight > 0.5) {
        // Light pass - multiplicative blending
        vec4 lightColor = texture(sampler2D(iLightTex, iLightSampler), texUV);
        float intensity = lightColor.r * iLightIntensity;
        fragColor = vec4(iLightColor * intensity, intensity);
    } else {
        // Regular texture pass
        vec4 mainColor = texture(sampler2D(iMainTex, iMainSampler), texUV);
        vec4 lightColor = texture(sampler2D(iLightTex, iLightSampler), texUV);
        
        // Apply lighting
        vec3 finalColor = mainColor.rgb * max(iAmbientLight, lightColor.r);
        
        // Keep original alpha for sprites
        fragColor = vec4(finalColor, mainColor.a);
    }
}
@end

@program program vs fs 