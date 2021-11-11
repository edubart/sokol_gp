@module effect
@ctype vec2 sgp_vec2
@ctype float float

@vs vs
layout(location=0) in vec4 coord;
layout(location=0) out vec2 texUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    texUV = coord.zw;
}
@end

@fs fs
layout(binding=0) uniform sampler2D iChannel0;
layout(binding=1) uniform sampler2D iChannel1;
uniform uniforms {
    vec2 iVelocity;
    float iPressure;
    float iTime;
    float iWarpiness;
    float iRatio;
    float iZoom;
    float iLevel;
};
in vec2 texUV;
out vec4 fragColor;
float noise(vec2 p) {
    return texture(iChannel1, p).r;
}
void main() {
    vec3 tex_col = texture(iChannel0, texUV).rgb;
    vec2 fog_uv = (texUV * vec2(iRatio, 1.0)) * iZoom;
    float f = noise(fog_uv - iVelocity*iTime);
    f = noise(fog_uv + f*iWarpiness);
    vec3 col = mix(tex_col, vec3(f), iPressure);
    fragColor = vec4(col, 1.0);
}
@end

@program program vs fs
