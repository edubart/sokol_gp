/* This is the shader used by the default SGP pipeline */
@module sgp

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
uniform texture2D iTexChannel0;
uniform sampler iSmpChannel0;
layout(location=0) in vec2 texUV;
layout(location=1) in vec4 iColor;
layout(location=0) out vec4 fragColor;
void main() {
    fragColor = texture(sampler2D(iTexChannel0, iSmpChannel0), texUV) * iColor;
}
@end

@program program vs fs
