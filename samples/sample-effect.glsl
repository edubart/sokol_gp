@vs effect_vs
layout(location=0) in vec4 coord;
layout(location=0) out vec2 fragUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragUV = coord.zw;
}
@end

@fs effect_fs
#if 0
uniform sampler2D iChannel0;
uniform effect_uniform {
    vec2 iResolution;
    float iLevel;
    float iTime;
};
in vec2 fragUV;
out vec4 fragColor;
void main() {
    vec4 orig_col = texture(iChannel0, fragUV);
    vec3 sepia_col = vec3(dot(orig_col.rgb, vec3(0.393, 0.769, 0.189)),
                          dot(orig_col.rgb, vec3(0.349, 0.686, 0.168)),
                          dot(orig_col.rgb, vec3(0.272, 0.534, 0.131)));
    vec4 col = vec4(mix(orig_col.rgb, sepia_col, iLevel), orig_col.a);
    fragColor = col;
}
#endif
layout(binding=0) uniform sampler2D iChannel0;
layout(binding=1) uniform sampler2D iChannel1;
uniform effect_uniform {
    vec2 iVelocity;
    float iPressure;
    float iTime;
    float iWarpiness;
    float iRatio;
    float iZoom;
    float iLevel;
};
in vec2 fragUV;
out vec4 fragColor;
float noise(vec2 p) {
    return texture(iChannel1, p).r;
}
void main() {
    vec3 tex_col = texture(iChannel0, fragUV).rgb;
    vec2 fog_uv = (fragUV * vec2(iRatio, 1.0)) * iZoom;
    float f = noise(fog_uv - iVelocity*iTime);
    f = noise(fog_uv + f*iWarpiness);
    vec3 col = mix(tex_col, vec3(f), iPressure);
    fragColor = vec4(col, 1.0);
}
@end

@program effect effect_vs effect_fs
