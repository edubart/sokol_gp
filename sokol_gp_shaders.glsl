@vs sgp_vs
in vec4 coord;
out vec2 fragUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragUV = coord.zw;
}
@end

@fs sgp_fs
uniform sampler2D iChannel0;
uniform vec4 iColor;
in vec2 fragUV;
out vec4 fragColor;
void main() {
    fragColor = texture(iChannel0, fragUV) * iColor;
}
@end

@program sgp sgp_vs sgp_fs
