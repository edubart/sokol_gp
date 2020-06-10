@vs sgp_vs
layout(location=0) in vec4 coord;
layout(location=0) out vec2 uv;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    uv = coord.zw;
}
@end

@fs sgp_fs
layout(binding=0) uniform sampler2D tex;
//uniform fs_in { vec4 color; };
uniform vec4 color;
layout(location=0) in vec2 uv;
layout(location=0) out vec4 frag_color;
void main() {
    frag_color = texture(tex, uv) * color;
}
@end

@program sgp sgp_vs sgp_fs
