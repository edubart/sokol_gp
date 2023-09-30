/* This is the shader used by the default SGP pipeline */
@module sgp

@vs vs
in vec4 coord;
out vec2 texUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    texUV = coord.zw;
}
@end

@fs fs
uniform texture2D iTexChannel0;
uniform sampler iSmpChannel0;
uniform fs_params {
    vec4 iColor;
};
in vec2 texUV;
out vec4 fragColor;
void main() {
    fragColor = texture(sampler2D(iTexChannel0, iSmpChannel0), texUV) * iColor;
}
@end

@program program vs fs
