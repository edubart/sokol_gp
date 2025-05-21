@module zelda_lighting
@ctype vec2 sgp_vec2
@ctype float float

@vs vs
layout(location=0) in vec4 coord;
layout(location=1) in vec4 color;
layout(location=0) out vec2 texUV;
layout(location=1) out vec4 iColor;
void main() {
    // Use constant z=0.0 since we don't need depth ordering within the scene
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    texUV = coord.zw;
    iColor = color;
}
@end

@fs fs
layout(binding=0) uniform texture2D iMainTex;    // Main scene texture or lightmap texture
layout(binding=1) uniform texture2D iLightMask;  // Light accumulation mask
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
        // Light pass - draws each individual light to the mask
        
        // Sample the lightmap to get the shape of the light
        vec4 lightMask = texture(sampler2D(iMainTex, iMainSampler), texUV);
        
        // Calculate falloff for soft circle light
        float dist = distance(texUV, vec2(0.5, 0.5));
        float attenuation = 1.0 - smoothstep(0.0, 0.48, dist);  // Adjusted for sharper edge
        
        // Combined light intensity
        float intensity = lightMask.r * iLightIntensity * attenuation;
        
        // Output light color with proper alpha for the mask
        fragColor = vec4(iLightColor * intensity, intensity);
    } else {
        // Scene pass - renders backgrounds and sprites with lighting
        vec4 mainColor = texture(sampler2D(iMainTex, iMainSampler), texUV);
        vec4 lightMask = texture(sampler2D(iLightMask, iLightSampler), texUV);
        
        // For masking approach: lightMask reveals the fully-lit scene underneath
        // Use maximum to ensure areas with light are fully visible
        float lighting = max(iAmbientLight, lightMask.r);
        
        // Apply lighting to the texture
        vec3 finalColor = mainColor.rgb * lighting;
        
        // Keep original alpha from texture
        fragColor = vec4(finalColor, mainColor.a);
    }
}
@end

@program program vs fs 