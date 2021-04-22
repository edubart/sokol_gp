@vs effect_vs
in vec4 coord;
out vec2 fragUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragUV = coord.zw;
}
@end

@fs effect_fs
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
@end

@program effect effect_vs effect_fs
