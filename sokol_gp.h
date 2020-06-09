/*
sokol_gp.h - minimal efficient cross platform 2D graphics painter
https://github.com/edubart/sokol_gp
*/

#ifndef SOKOL_GP_INCLUDED
#define SOKOL_GP_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gp.h"
#endif

#ifndef SGP_BATCH_OPTIMIZER_DEPTH
#define SGP_BATCH_OPTIMIZER_DEPTH 8
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sgp_error {
    SGP_NO_ERROR = 0,
    SGP_ERROR_SOKOL_INVALID,
    SGP_ERROR_VERTICES_FULL,
    SGP_ERROR_UNIFORMS_FULL,
    SGP_ERROR_COMMANDS_FULL,
    SGP_ERROR_TRANSFORM_STACK_OVERFLOW,
    SGP_ERROR_TRANSFORM_STACK_UNDERFLOW,
    SGP_ERROR_STATE_STACK_OVERFLOW,
    SGP_ERROR_STATE_STACK_UNDERFLOW,
    SGP_ERROR_ALLOC_FAILED,
    SGP_ERROR_MAKE_BUFFER_FAILED,
    SGP_ERROR_MAKE_IMAGE_FAILED,
    SGP_ERROR_MAKE_SHADER_FAILED,
    SGP_ERROR_MAKE_PIPELINE_FAILED
} sgp_error;

typedef enum sgp_blend_mode {
    SGP_BLENDMODE_NONE = 0, /* no blending
                               dstRGBA = srcRGBA */
    SGP_BLENDMODE_BLEND,    /* alpha blending
                               dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                               dstA = srcA + (dstA * (1-srcA)) */
    SGP_BLENDMODE_ADD,      /* additive blending
                               dstRGB = (srcRGB * srcA) + dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MOD,      /* color modulate
                               dstRGB = srcRGB * dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MUL,      /* color multiply
                               dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                               dstA = (srcA * dstA) + (dstA * (1-srcA)) */
    _SGP_BLENDMODE_NUM
} sgp_blend_mode;

typedef struct sgp_isize {
    int w, h;
} sgp_isize;

typedef struct sgp_irect {
    int x, y, w, h;
} sgp_irect;

typedef struct sgp_rect {
    float x, y, w, h;
} sgp_rect;

typedef struct sgp_textured_rect {
    sgp_rect dst;
    sgp_rect src;
} sgp_textured_rect;

typedef struct sgp_vec2 {
    float x, y;
} sgp_vec2;

typedef sgp_vec2 sgp_point;

typedef struct sgp_line {
    sgp_point a, b;
} sgp_line;

typedef struct sgp_triangle {
    sgp_point a, b, c;
} sgp_triangle;

typedef struct sgp_mat3 {
    float v[3][3];
} sgp_mat3;

typedef struct sgp_color {
    float r, g, b, a;
} sgp_color;

typedef struct sgp_desc {
    int max_vertices;
    int max_commands;
} sgp_desc;

typedef struct sgp_uniform {
    sgp_color color;
} sgp_uniform;

typedef struct sgp_state {
    sgp_isize frame_size;
    sgp_irect viewport;
    sgp_irect scissor;
    sgp_mat3 proj;
    sgp_mat3 transform;
    sgp_mat3 mvp;
    sgp_uniform uniform;
    sgp_blend_mode blend_mode;
    uint32_t _base_vertex;
    uint32_t _base_uniform;
    uint32_t _base_command;
} sgp_state;

// setup functions
SOKOL_API_DECL bool sgp_setup(const sgp_desc* desc);
SOKOL_API_DECL void sgp_shutdown();
SOKOL_API_DECL bool sgp_is_valid();
SOKOL_API_DECL sgp_error sgp_get_error_code();
SOKOL_API_DECL const char* sgp_get_error();

// rendering functions
SOKOL_API_DECL void sgp_begin(int width, int height);
SOKOL_API_DECL void sgp_flush();
SOKOL_API_DECL void sgp_end();

// state projection functions
SOKOL_API_DECL void sgp_ortho(float left, float right, float top, float bottom);
SOKOL_API_DECL void sgp_reset_proj();

// state transform functions
SOKOL_API_DECL void sgp_push_transform();
SOKOL_API_DECL void sgp_pop_transform();
SOKOL_API_DECL void sgp_reset_transform();
SOKOL_API_DECL void sgp_translate(float x, float y);
SOKOL_API_DECL void sgp_rotate(float theta);
SOKOL_API_DECL void sgp_rotate_at(float theta, float x, float y);
SOKOL_API_DECL void sgp_scale(float sx, float sy);
SOKOL_API_DECL void sgp_scale_at(float sx, float sy, float x, float y);

// state change functions
SOKOL_API_DECL void sgp_set_blend_mode(sgp_blend_mode blend_mode);
SOKOL_API_DECL void sgp_reset_blend_mode();
SOKOL_API_DECL void sgp_set_color(float r, float g, float b, float a);
SOKOL_API_DECL void sgp_reset_color();
SOKOL_API_DECL void sgp_viewport(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_viewport();
SOKOL_API_DECL void sgp_scissor(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_scissor();
SOKOL_API_DECL void sgp_reset_state();

// drawing functions
SOKOL_API_DECL void sgp_clear();
SOKOL_API_DECL void sgp_draw_points(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_point(float x, float y);
SOKOL_API_DECL void sgp_draw_lines(const sgp_line* lines, uint32_t count);
SOKOL_API_DECL void sgp_draw_line(float ax, float ay, float bx, float by);
SOKOL_API_DECL void sgp_draw_line_strip(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy);
SOKOL_API_DECL void sgp_draw_filled_triangle_strip(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_rect(float x, float y, float w, float h);
SOKOL_API_DECL void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_textured_rect(sg_image image, float x, float y, float w, float h);
SOKOL_API_DECL void sgp_draw_textured_rects_ex(sg_image image, const sgp_textured_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_textured_rect_ex(sg_image image, sgp_rect dest_rect, sgp_rect src_rect);

// querying functions
SOKOL_API_DECL sgp_state* sgp_query_state();
SOKOL_API_DECL sgp_desc sgp_query_desc();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GP_INCLUDED

#ifdef SOKOL_GP_IMPL
#ifndef SOKOL_GP_IMPL_INCLUDED
#define SOKOL_GP_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#include <string.h>
#include <math.h>
#include <stddef.h>

#ifndef SOKOL_LIKELY
#ifdef __GNUC__
#define SOKOL_LIKELY(x) __builtin_expect(x, 1)
#define SOKOL_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define SOKOL_LIKELY(x) (x)
#define SOKOL_UNLIKELY(x) (x)
#endif
#endif

enum {
    _SGP_INIT_COOKIE = 0xCAFED0D,
    _SGP_DEFAULT_MAX_VERTICES = 65536,
    _SGP_DEFAULT_MAX_COMMANDS = 16384,
    _SGP_MAX_STACK_DEPTH = 64,
};

typedef struct _sgp_vertex {
    sgp_vec2 position;
    sgp_vec2 texcoord;
} _sgp_vertex;

typedef struct _sgp_region {
    float x1, y1, x2, y2;
} _sgp_region;

typedef struct _sgp_draw_args {
    sg_pipeline pip;
    sg_image img;
    _sgp_region region;
    uint32_t uniform_index;
    uint32_t vertex_index;
    uint32_t num_vertices;
} _sgp_draw_args;

typedef union _sgp_command_args {
    _sgp_draw_args draw;
    sgp_irect viewport;
    sgp_irect scissor;
} _sgp_command_args;

typedef enum _sgp_command_type {
    SGP_COMMAND_NONE = 0,
    SGP_COMMAND_DRAW,
    SGP_COMMAND_VIEWPORT,
    SGP_COMMAND_SCISSOR
} _sgp_command_type;

typedef struct _sgp_command {
    _sgp_command_type cmd;
    _sgp_command_args args;
} _sgp_command;

typedef struct _sgp_context {
    uint32_t init_cookie;
    const char *last_error_message;
    sgp_error last_error_code;
    sgp_desc desc;

    // resources
    sg_shader shader;
    sg_buffer vertex_buf;
    sg_image white_img;
    sg_pipeline pipelines[_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM];

    // command queue
    uint32_t cur_vertex;
    uint32_t cur_uniform;
    uint32_t cur_command;
    uint32_t num_vertices;
    uint32_t num_uniforms;
    uint32_t num_commands;
    _sgp_vertex* vertices;
    sgp_uniform* uniforms;
    _sgp_command* commands;

    // state tracking
    sgp_state state;

    // matrix stack
    uint32_t cur_transform;
    uint32_t cur_state;
    sgp_mat3 transform_stack[_SGP_MAX_STACK_DEPTH];
    sgp_state state_stack[_SGP_MAX_STACK_DEPTH];
} _sgp_context;

static _sgp_context _sgp;

static const sgp_mat3 _sgp_mat3_identity = {{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
}};

static void _sgp_set_error(sgp_error code, const char *message) {
    _sgp.last_error_code = code;
    _sgp.last_error_message = message;
    SOKOL_LOG(message);
}

#if defined(SOKOL_GLCORE33)
static const char* _sgp_vs_source =
"#version 330\n"
"layout(location=0) in vec4 coord;\n"
"out vec2 uv;\n"
"void main() {\n"
"    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"    uv = coord.zw;\n"
"}\n";
static const char* _sgp_fs_source =
"#version 330\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"    frag_color = texture(tex, uv) * color;\n"
"}\n";
#elif defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
static const char* _sgp_vs_source =
"attribute vec4 coord;\n"
"varying vec2 uv;\n"
"void main() {\n"
"    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"    uv = coord.zw;\n"
"}\n";
static const char* _sgp_fs_source =
"precision mediump float;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"varying vec2 uv;\n"
"void main() {\n"
"    gl_FragColor = texture2D(tex, uv) * color;\n"
"}\n";
#elif defined(SOKOL_D3D11)
static const char* _sgp_vs_source =
"struct vs_out {\n"
"    float2 uv: TEXCOORD0;\n"
"    float4 pos: SV_Position;\n"
"};\n"
"vs_out main(float4 coord: POSITION) {\n"
"    vs_out outp;\n"
"    outp.pos = float4(coord.xy, 0.0f, 1.0f);\n"
"    outp.uv = coord.zw;\n"
"    return outp;\n"
"}\n";
static const char* _sgp_fs_source =
"Texture2D<float4> tex: register(t0);\n"
"SamplerState smp: register(s0);\n"
"uniform float4 color;\n"
"float4 main(float2 uv: TEXCOORD0): SV_Target0 {\n"
"    return tex.Sample(smp, uv) * color;\n"
"}\n";
#elif defined(SOKOL_METAL)
static const char* _sgp_vs_source =
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"using namespace metal;\n"
"struct main0_out {\n"
"    float2 uv [[user(locn0)]];\n"
"    float4 gl_Position [[position]];\n"
"};\n"
"struct main0_in {\n"
"    float4 coord [[attribute(0)]];\n"
"};\n"
"vertex main0_out _main(main0_in in [[stage_in]]) {\n"
"    main0_out out = {};\n"
"    out.gl_Position = float4(in.coord.xy, 0.0, 1.0);\n"
"    out.uv = in.coord.zw;\n"
"    return out;\n"
"}\n";
static const char* _sgp_fs_source =
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"using namespace metal;\n"
"struct main0_out {\n"
"    float4 frag_color [[color(0)]];\n"
"};\n"
"struct main0_in {\n"
"    float2 uv [[user(locn0)]];\n"
"};\n"
"fragment main0_out _main(main0_in in [[stage_in]], constant float4& color [[buffer(0)]], texture2d<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]]) {\n"
"    main0_out out = {};\n"
"    out.frag_color = tex.sample(texSmplr, in.uv) * color;\n"
"    return out;\n"
"}\n";
#elif defined(SOKOL_WGPU)
#define _SGP_BYTECODE
/*
    #version 450
    layout(location = 0) in vec4 coord;
    layout(location = 0) out vec2 uv;
    void main() {
        gl_Position = vec4(coord.xy, 0.0, 1.0);
        uv = coord.zw;
    }
*/
static const uint8_t _sgp_vs_bytecode[1160] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x08,0x00,0x08,0x00,0x21,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,
    0x02,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x0f,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,
    0x07,0x00,0x03,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x37,0x00,
    0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x2f,0x2f,0x20,0x4f,
    0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,
    0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x30,
    0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,
    0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,0x6f,0x70,
    0x65,0x6e,0x67,0x6c,0x31,0x30,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,
    0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,
    0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x2e,
    0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,
    0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,
    0x76,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,
    0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x65,0x6e,
    0x74,0x72,0x79,0x2d,0x70,0x6f,0x69,0x6e,0x74,0x20,0x6d,0x61,0x69,0x6e,0x0a,0x23,
    0x6c,0x69,0x6e,0x65,0x20,0x31,0x0a,0x00,0x05,0x00,0x04,0x00,0x05,0x00,0x00,0x00,
    0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x05,0x00,0x06,0x00,0x0c,0x00,0x00,0x00,
    0x67,0x6c,0x5f,0x50,0x65,0x72,0x56,0x65,0x72,0x74,0x65,0x78,0x00,0x00,0x00,0x00,
    0x06,0x00,0x06,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,
    0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x00,0x06,0x00,0x07,0x00,0x0c,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,0x6f,0x69,0x6e,0x74,0x53,0x69,0x7a,0x65,
    0x00,0x00,0x00,0x00,0x06,0x00,0x07,0x00,0x0c,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x67,0x6c,0x5f,0x43,0x6c,0x69,0x70,0x44,0x69,0x73,0x74,0x61,0x6e,0x63,0x65,0x00,
    0x06,0x00,0x07,0x00,0x0c,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x67,0x6c,0x5f,0x43,
    0x75,0x6c,0x6c,0x44,0x69,0x73,0x74,0x61,0x6e,0x63,0x65,0x00,0x05,0x00,0x03,0x00,
    0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x12,0x00,0x00,0x00,
    0x63,0x6f,0x6f,0x72,0x64,0x00,0x00,0x00,0x05,0x00,0x03,0x00,0x1e,0x00,0x00,0x00,
    0x75,0x76,0x00,0x00,0x48,0x00,0x05,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x0c,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x48,0x00,0x05,0x00,
    0x0c,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x48,0x00,0x05,0x00,0x0c,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,
    0x04,0x00,0x00,0x00,0x47,0x00,0x03,0x00,0x0c,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x47,0x00,0x04,0x00,0x12,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x47,0x00,0x04,0x00,0x1e,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x13,0x00,0x02,0x00,0x03,0x00,0x00,0x00,0x21,0x00,0x03,0x00,0x04,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x16,0x00,0x03,0x00,0x07,0x00,0x00,0x00,0x20,0x00,0x00,0x00,
    0x17,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
    0x15,0x00,0x04,0x00,0x09,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x2b,0x00,0x04,0x00,0x09,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x1c,0x00,0x04,0x00,0x0b,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,
    0x1e,0x00,0x06,0x00,0x0c,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x07,0x00,0x00,0x00,
    0x0b,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x0d,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x0d,0x00,0x00,0x00,
    0x0e,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x15,0x00,0x04,0x00,0x0f,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x0f,0x00,0x00,0x00,
    0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x11,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x11,0x00,0x00,0x00,
    0x12,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x17,0x00,0x04,0x00,0x13,0x00,0x00,0x00,
    0x07,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x07,0x00,0x00,0x00,
    0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x07,0x00,0x00,0x00,
    0x17,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x20,0x00,0x04,0x00,0x1b,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x1d,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x1d,0x00,0x00,0x00,
    0x1e,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x36,0x00,0x05,0x00,0x03,0x00,0x00,0x00,
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0xf8,0x00,0x02,0x00,
    0x06,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x14,0x00,0x00,0x00,
    0x12,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x07,0x00,0x00,0x00,0x18,0x00,0x00,0x00,
    0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x07,0x00,0x00,0x00,
    0x19,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x50,0x00,0x07,0x00,
    0x08,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x19,0x00,0x00,0x00,
    0x16,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x1b,0x00,0x00,0x00,
    0x1c,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x3e,0x00,0x03,0x00,
    0x1c,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x4f,0x00,0x07,0x00,0x13,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x3e,0x00,0x03,0x00,0x1e,0x00,0x00,0x00,0x20,0x00,0x00,0x00,
    0xfd,0x00,0x01,0x00,0x38,0x00,0x01,0x00,
};
/*
    #version 450
    layout(set = 0, binding = 4, std140) uniform fs_in {
        vec4 color;
    } in;
    layout(location = 0, set = 2, binding = 0) uniform sampler2D tex;
    layout(location = 0) out vec4 frag_color;
    layout(location = 0) in vec2 uv;
    void main() {
        frag_color = texture(tex, uv) * in.color;
    }
*/
static const uint8_t _sgp_fs_bytecode[1060] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x08,0x00,0x08,0x00,0x1e,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,
    0x02,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x0f,0x00,0x07,0x00,0x04,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,
    0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x10,0x00,0x03,0x00,
    0x05,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0x03,0x00,0x01,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x03,0x00,0x37,0x00,0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,
    0x01,0x00,0x00,0x00,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,
    0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,
    0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x30,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,
    0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,
    0x6c,0x69,0x65,0x6e,0x74,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,0x31,0x30,0x30,0x0a,
    0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,
    0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,
    0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x2e,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,
    0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,
    0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,
    0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,
    0x65,0x73,0x73,0x65,0x64,0x20,0x65,0x6e,0x74,0x72,0x79,0x2d,0x70,0x6f,0x69,0x6e,
    0x74,0x20,0x6d,0x61,0x69,0x6e,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x0a,0x00,
    0x05,0x00,0x04,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,
    0x05,0x00,0x05,0x00,0x0a,0x00,0x00,0x00,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,
    0x6f,0x72,0x00,0x00,0x05,0x00,0x03,0x00,0x0e,0x00,0x00,0x00,0x74,0x65,0x78,0x00,
    0x05,0x00,0x03,0x00,0x12,0x00,0x00,0x00,0x75,0x76,0x00,0x00,0x05,0x00,0x04,0x00,
    0x15,0x00,0x00,0x00,0x66,0x73,0x5f,0x69,0x6e,0x00,0x00,0x00,0x06,0x00,0x05,0x00,
    0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x6f,0x6c,0x6f,0x72,0x00,0x00,0x00,
    0x05,0x00,0x03,0x00,0x17,0x00,0x00,0x00,0x5f,0x32,0x33,0x00,0x47,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x12,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x05,0x00,
    0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x47,0x00,0x03,0x00,0x15,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x17,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x17,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x13,0x00,0x02,0x00,
    0x03,0x00,0x00,0x00,0x21,0x00,0x03,0x00,0x04,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x16,0x00,0x03,0x00,0x07,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x08,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x09,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x09,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x19,0x00,0x09,0x00,
    0x0b,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x1b,0x00,0x03,0x00,0x0c,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x0d,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x10,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x11,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x11,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x1e,0x00,0x03,0x00,
    0x15,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x16,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x16,0x00,0x00,0x00,
    0x17,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x15,0x00,0x04,0x00,0x18,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x18,0x00,0x00,0x00,
    0x19,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x1a,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x36,0x00,0x05,0x00,0x03,0x00,0x00,0x00,
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0xf8,0x00,0x02,0x00,
    0x06,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x0c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,
    0x0e,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x10,0x00,0x00,0x00,0x13,0x00,0x00,0x00,
    0x12,0x00,0x00,0x00,0x57,0x00,0x05,0x00,0x08,0x00,0x00,0x00,0x14,0x00,0x00,0x00,
    0x0f,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x1a,0x00,0x00,0x00,
    0x1b,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x19,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,
    0x08,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0x1b,0x00,0x00,0x00,0x85,0x00,0x05,0x00,
    0x08,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,
    0x3e,0x00,0x03,0x00,0x0a,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,0xfd,0x00,0x01,0x00,
    0x38,0x00,0x01,0x00,
};
#elif defined(SOKOL_DUMMY_BACKEND)
static const char _sgp_vs_source[] = "";
static const char _sgp_fs_source[] = "";
#else
#error "Please define one of SOKOL_GLCORE33, SOKOL_GLES2, SOKOL_GLES3, SOKOL_D3D11, SOKOL_METAL, SOKOL_WGPU or SOKOL_DUMMY_BACKEND!"
#endif

static sg_pipeline _sgp_lookup_pipeline(sg_primitive_type prim_type, sgp_blend_mode blend_mode) {
    uint32_t pip_index = (prim_type * _SGP_BLENDMODE_NUM) +
                         blend_mode;
    if(_sgp.pipelines[pip_index].id != SG_INVALID_ID)
        return _sgp.pipelines[pip_index];

    sg_blend_state blend;
    switch(blend_mode) {
        case SGP_BLENDMODE_NONE:
            blend = (sg_blend_state) {
                .enabled = false,
                .src_factor_rgb = SG_BLENDFACTOR_ONE,
                .dst_factor_rgb = SG_BLENDFACTOR_ZERO,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ZERO,
            };
            break;
        case SGP_BLENDMODE_BLEND:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            };
            break;
        case SGP_BLENDMODE_ADD:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE,
                .src_factor_alpha = SG_BLENDFACTOR_ZERO,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE,
            };
            break;
        case SGP_BLENDMODE_MOD:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_DST_COLOR,
                .dst_factor_rgb = SG_BLENDFACTOR_ZERO,
                .src_factor_alpha = SG_BLENDFACTOR_ZERO,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE,

            };
            break;
        case SGP_BLENDMODE_MUL:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_DST_COLOR,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_DST_ALPHA,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            };
            break;
        default:
            blend = (sg_blend_state) {.enabled = false};
            SOKOL_UNREACHABLE;
            break;
    }
    blend.color_format = _sg.desc.context.color_format;
    blend.depth_format = _sg.desc.context.depth_format;

    // create pipeline
    sg_pipeline_desc pip_desc = {
        .layout = {
            .buffers = {
                {.stride=sizeof(_sgp_vertex)},
            },
            .attrs = {
                {.offset=0, .format=SG_VERTEXFORMAT_FLOAT4},
            },
        },
        .shader = _sgp.shader,
        .primitive_type = prim_type,
        .blend = blend
    };
    sg_pipeline pip = sg_make_pipeline(&pip_desc);
    if(pip.id == SG_INVALID_ID)
        _sgp_set_error(SGP_ERROR_MAKE_PIPELINE_FAILED, "SGP failed to create common pipeline");
    _sgp.pipelines[pip_index] = pip;
    return pip;
};

bool sgp_setup(const sgp_desc* desc) {
    SOKOL_ASSERT(_sgp.init_cookie == 0);

    // init
    _sgp.init_cookie = _SGP_INIT_COOKIE;
    _sgp.last_error_code = SGP_NO_ERROR;
    _sgp.last_error_message = "";

    // set desc default values
    _sgp.desc = *desc;
    _sgp.desc.max_vertices = _sg_def(desc->max_vertices, _SGP_DEFAULT_MAX_VERTICES);
    _sgp.desc.max_commands = _sg_def(desc->max_commands, _SGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    _sgp.num_vertices = _sgp.desc.max_vertices;
    _sgp.num_commands = _sgp.desc.max_commands;
    _sgp.num_uniforms = _sgp.desc.max_commands;
    _sgp.vertices = (_sgp_vertex*) SOKOL_MALLOC(_sgp.num_vertices * sizeof(_sgp_vertex));
    _sgp.uniforms = (sgp_uniform*) SOKOL_MALLOC(_sgp.num_uniforms * sizeof(sgp_uniform));
    _sgp.commands = (_sgp_command*) SOKOL_MALLOC(_sgp.num_commands * sizeof(_sgp_command));
    if(!_sgp.commands || !_sgp.uniforms || !_sgp.commands) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_ALLOC_FAILED, "SGP failed to allocate buffers");
        return false;
    }
    memset(_sgp.vertices, 0, _sgp.num_vertices * sizeof(_sgp_vertex));
    memset(_sgp.uniforms, 0, _sgp.num_uniforms * sizeof(sgp_uniform));
    memset(_sgp.commands, 0, _sgp.num_commands * sizeof(_sgp_command));

    // create vertex buffer
    sg_buffer_desc vertex_buf_desc = {
        .size = (int)(_sgp.num_vertices * sizeof(_sgp_vertex)),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
    };
    _sgp.vertex_buf = sg_make_buffer(&vertex_buf_desc);
    if(_sgp.vertex_buf.id == SG_INVALID_ID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_BUFFER_FAILED, "SGP failed to create vertex buffer");
        return false;
    }

    // create white texture
    uint32_t pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));
    sg_image_desc white_img_desc = {
        .type = SG_IMAGETYPE_2D,
        .width = 2, .height = 2,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .content = {.subimage = {{{.ptr = pixels, .size = sizeof(pixels)}}}},
        .label = "sgp-white-texture",
    };
    _sgp.white_img = sg_make_image(&white_img_desc);
    if(_sgp.white_img.id == SG_INVALID_ID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_IMAGE_FAILED, "SGP failed to create white image");
        return false;
    }

    // create common shader
    sg_shader_desc shader_desc = {
        .attrs = {
            {.name="coord", .sem_name="POSITION", .sem_index=0},
        },
        .vs = {
#ifdef _SGP_BYTECODE
            .byte_code = _sgp_vs_bytecode,
            .byte_code_size = sizeof(_sgp_vs_bytecode),
#else
            .source = _sgp_vs_source,
#endif
        },
        .fs = {
#ifdef _SGP_BYTECODE
            .byte_code = _sgp_fs_bytecode,
            .byte_code_size = sizeof(_sgp_fs_bytecode),
#else
            .source = _sgp_fs_source,
#endif
            .uniform_blocks = {{
                .size = sizeof(sgp_uniform),
                .uniforms = {{.name="color", .type=SG_UNIFORMTYPE_FLOAT4}},
            }},
            .images = {{.name = "tex", .type=SG_IMAGETYPE_2D}},
        },
    };
    _sgp.shader = sg_make_shader(&shader_desc);
    if(_sgp.shader.id == SG_INVALID_ID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_SHADER_FAILED, "SGP failed to create common shader");
        return false;
    }

    // create common pipelines
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_BLEND);
    if(_sgp.last_error_code != SGP_NO_ERROR) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_PIPELINE_FAILED, "SGP failed to create common pipeline");
        return false;
    }

    return true;
}

void sgp_shutdown() {
    if(_sgp.init_cookie == 0) return; // not initialized
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state == 0);
    if(_sgp.vertices)
        SOKOL_FREE(_sgp.vertices);
    if(_sgp.uniforms)
        SOKOL_FREE(_sgp.uniforms);
    if(_sgp.commands)
        SOKOL_FREE(_sgp.commands);
    for(uint32_t i=0;i<_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM;++i) {
        sg_pipeline pip = _sgp.pipelines[i];
        if(pip.id != SG_INVALID_ID)
            sg_destroy_pipeline(pip);
    }
    if(_sgp.shader.id != SG_INVALID_ID)
        sg_destroy_shader(_sgp.shader);
    if(_sgp.vertex_buf.id != SG_INVALID_ID)
        sg_destroy_buffer(_sgp.vertex_buf);
    if(_sgp.white_img.id != SG_INVALID_ID)
        sg_destroy_image(_sgp.white_img);
    _sgp = (_sgp_context){.init_cookie=0};
}

bool sgp_is_valid() {
    return _sgp.init_cookie == _SGP_INIT_COOKIE;
}

sgp_error sgp_get_error_code() {
    return _sgp.last_error_code;
}

const char* sgp_get_error() {
    return _sgp.last_error_message;
}

static inline sgp_mat3 _sgp_default_proj(int width, int height) {
    // matrix to convert screen coordinate system
    // to the usual the coordinate system used on the backends
    return (sgp_mat3){{
        {2.0f/width,           0.0f, -1.0f},
        {      0.0f,   -2.0f/height,  1.0f},
        {      0.0f,           0.0f,  1.0f}
    }};
}

void sgp_begin(int width, int height) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_OVERFLOW, "SGP state stack overflow");
        return;
    }

    // begin reset last error
    _sgp.last_error_message = "";
    _sgp.last_error_code = SGP_NO_ERROR;

    // save current state
    _sgp.state_stack[_sgp.cur_state++] = _sgp.state;

    // reset to default state
    _sgp.state.frame_size = (sgp_isize){width, height};
    _sgp.state.viewport = (sgp_irect){0, 0, width, height};
    _sgp.state.scissor = (sgp_irect){0, 0, -1, -1};
    _sgp.state.proj = _sgp_default_proj(width, height);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp.state.proj;
    _sgp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
    _sgp.state.blend_mode = SGP_BLENDMODE_NONE;
    _sgp.state._base_vertex = _sgp.cur_vertex;
    _sgp.state._base_uniform = _sgp.cur_uniform;
    _sgp.state._base_command = _sgp.cur_command;
}

void sgp_flush() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // draw nothing on errors
    if(_sgp.last_error_code != SGP_NO_ERROR)
        return;

    // nothing to be drawn
    if(_sgp.cur_command <= _sgp.state._base_command)
        return;

    // flush commands
    const uint32_t SG_IMPOSSIBLE_ID = 0xffffffffU;
    uint32_t cur_pip_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_img_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_uniform_index = SG_IMPOSSIBLE_ID;
    uint32_t cur_base_vertex = 0;
    uint32_t base_vertex = _sgp.state._base_vertex;
    uint32_t size_vertices = (_sgp.cur_vertex - base_vertex) * sizeof(_sgp_vertex);
    int offset = sg_append_buffer(_sgp.vertex_buf, &_sgp.vertices[base_vertex], size_vertices);

    // define the resource bindings
    sg_bindings bind = (sg_bindings){
        .vertex_buffers = {_sgp.vertex_buf},
        .vertex_buffer_offsets = {offset},
    };

    for(uint32_t i = _sgp.state._base_command; i < _sgp.cur_command; ++i) {
        _sgp_command* cmd = &_sgp.commands[i];
        switch(cmd->cmd) {
            case SGP_COMMAND_VIEWPORT: {
                sgp_irect* args = &cmd->args.viewport;
                sg_apply_viewport(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_SCISSOR: {
                sgp_irect* args = &cmd->args.scissor;
                sg_apply_scissor_rect(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_DRAW: {
                _sgp_draw_args* args = &cmd->args.draw;
                if(args->num_vertices == 0)
                    break;
                if(args->pip.id != cur_pip_id) {
                    // when pipeline changes, also need to re-apply uniforms and bindings
                    cur_img_id = SG_IMPOSSIBLE_ID;
                    cur_uniform_index = SG_IMPOSSIBLE_ID;
                    cur_pip_id = args->pip.id;
                    sg_apply_pipeline(args->pip);
                }
                if(cur_img_id != args->img.id) {
                    cur_base_vertex = base_vertex;
                    cur_img_id = args->img.id;
                    bind.fs_images[0] = args->img;
                    sg_apply_bindings(&bind);
                }
                if(cur_uniform_index != args->uniform_index) {
                    cur_uniform_index = args->uniform_index;
                    sgp_uniform* uniform = &_sgp.uniforms[cur_uniform_index];
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, uniform, sizeof(sgp_uniform));
                }
                sg_draw(args->vertex_index - cur_base_vertex, args->num_vertices, 1);
                break;
            }
            case SGP_COMMAND_NONE: {
                // this command was optimized away
                break;
            }
            default: {
                SOKOL_UNREACHABLE;
                break;
            }
        }
    }

    // rewind indexes
    _sgp.cur_vertex = _sgp.state._base_vertex;
    _sgp.cur_uniform = _sgp.state._base_uniform;
    _sgp.cur_command = _sgp.state._base_command;
}

void sgp_end() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state <= 0)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_UNDERFLOW, "SGP state stack underflow");
        return;
    }

    // restore old state
    _sgp.state = _sgp.state_stack[--_sgp.cur_state];
}

static inline sgp_mat3 _sgp_mat3_mul(const sgp_mat3* a, const sgp_mat3* b) {
    sgp_mat3 p;
    p.v[0][0] = a->v[0][0]*b->v[0][0] + a->v[0][1]*b->v[1][0] + a->v[0][2]*b->v[2][0];
    p.v[0][1] = a->v[0][0]*b->v[0][1] + a->v[0][1]*b->v[1][1] + a->v[0][2]*b->v[2][1];
    p.v[0][2] = a->v[0][0]*b->v[0][2] + a->v[0][1]*b->v[1][2] + a->v[0][2]*b->v[2][2];
    p.v[1][0] = a->v[1][0]*b->v[0][0] + a->v[1][1]*b->v[1][0] + a->v[1][2]*b->v[2][0];
    p.v[1][1] = a->v[1][0]*b->v[0][1] + a->v[1][1]*b->v[1][1] + a->v[1][2]*b->v[2][1];
    p.v[1][2] = a->v[1][0]*b->v[0][2] + a->v[1][1]*b->v[1][2] + a->v[1][2]*b->v[2][2];
    p.v[2][0] = a->v[2][0]*b->v[0][0] + a->v[2][1]*b->v[1][0] + a->v[2][2]*b->v[2][0];
    p.v[2][1] = a->v[2][0]*b->v[0][1] + a->v[2][1]*b->v[1][1] + a->v[2][2]*b->v[2][1];
    p.v[2][2] = a->v[2][0]*b->v[0][2] + a->v[2][1]*b->v[1][2] + a->v[2][2]*b->v[2][2];
    return p;
}

static sgp_mat3 _sgp_mul_proj_transform(sgp_mat3* proj, sgp_mat3* transform) {
    // this actually multiply matrix projection and transform matrix in an optimized way
    // the effect is the same as "return _sgp_mat3_mul(proj, transform);"
    float x = proj->v[0][0], y = proj->v[1][1];
    float tx = proj->v[0][2], ty = proj->v[1][2];
    float a = transform->v[2][0], b = transform->v[2][1], c = transform->v[2][2];
    return (sgp_mat3) {{
        {x*transform->v[0][0]+a*tx, x*transform->v[0][1]+b*tx, x*transform->v[0][2]+c*tx},
        {y*transform->v[1][0]+a*ty, y*transform->v[1][1]+b*ty, y*transform->v[1][2]+c*ty},
        {a, b, c}
    }};
}

void sgp_ortho(float left, float right, float top, float bottom) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float w = right - left;
    float h = top - bottom;
    _sgp.state.proj = (sgp_mat3){{
        {2.0f/w,   0.0f,  -(right+left)/w},
        {0.0f,   2.0f/h,  -(top+bottom)/h},
        {0.0f,     0.0f,             1.0f}
    }};
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_proj() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.proj = _sgp_default_proj(_sgp.state.viewport.w, _sgp.state.viewport.h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_push_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_OVERFLOW, "SGP transform stack overflow");
        return;
    }
    _sgp.transform_stack[_sgp.cur_transform++] = _sgp.state.transform;
}

void sgp_pop_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform <= 0)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_UNDERFLOW, "SGP transform stack underflow");
        return;
    }
    _sgp.state.transform = _sgp.transform_stack[--_sgp.cur_transform];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_translate(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by translate matrix:
    // 1.0f, 0.0f,    x,
    // 0.0f, 1.0f,    y,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][2] += x*_sgp.state.transform.v[0][0] + y*_sgp.state.transform.v[0][1];
    _sgp.state.transform.v[1][2] += x*_sgp.state.transform.v[1][0] + y*_sgp.state.transform.v[1][1];
    _sgp.state.transform.v[2][2] += x*_sgp.state.transform.v[2][0] + y*_sgp.state.transform.v[2][1];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate(float theta) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float sint = sinf(theta), cost = cosf(theta);
    // multiply by rotation matrix:
    // sint,  cost, 0.0f,
    // cost, -sint, 0.0f,
    // 0.0f,  0.0f, 1.0f,
    _sgp.state.transform = (sgp_mat3){{
       {sint*_sgp.state.transform.v[0][0]+cost*_sgp.state.transform.v[0][1], cost*_sgp.state.transform.v[0][0]-sint*_sgp.state.transform.v[0][1], _sgp.state.transform.v[0][2]},
       {sint*_sgp.state.transform.v[1][0]+cost*_sgp.state.transform.v[1][1], cost*_sgp.state.transform.v[1][0]-sint*_sgp.state.transform.v[1][1], _sgp.state.transform.v[1][2]},
       {sint*_sgp.state.transform.v[2][0]+cost*_sgp.state.transform.v[2][1], cost*_sgp.state.transform.v[2][0]-sint*_sgp.state.transform.v[2][1], _sgp.state.transform.v[2][2]}
    }};
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate_at(float theta, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_rotate(theta);
    sgp_translate(-x, -y);
}

void sgp_scale(float sx, float sy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by scale matrix:
    //   sx, 0.0f, 0.0f,
    // 0.0f,   sy, 0.0f,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][0] *= sx;
    _sgp.state.transform.v[1][0] *= sx;
    _sgp.state.transform.v[2][0] *= sx;
    _sgp.state.transform.v[0][1] *= sy;
    _sgp.state.transform.v[1][1] *= sy;
    _sgp.state.transform.v[2][1] *= sy;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_scale_at(float sx, float sy, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_scale(sx, sy);
    sgp_translate(-x, -y);
}

void sgp_set_blend_mode(sgp_blend_mode blend_mode) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp.state.blend_mode = blend_mode;
}

void sgp_reset_blend_mode() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sgp_set_blend_mode(SGP_BLENDMODE_NONE);
}

void sgp_set_color(float r, float g, float b, float a) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.uniform.color = (sgp_color){r,g,b,a};
}

void sgp_reset_color() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
}

static _sgp_vertex* _sgp_next_vertices(uint32_t count) {
    if(SOKOL_LIKELY(_sgp.cur_vertex + count <= _sgp.num_vertices)) {
        _sgp_vertex *vertices = &_sgp.vertices[_sgp.cur_vertex];
        _sgp.cur_vertex += count;
        return vertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL, "SGP vertices buffer is full");
        return NULL;
    }
}

static sgp_uniform* _sgp_prev_uniform() {
    if(SOKOL_LIKELY(_sgp.cur_uniform > 0)) {
        return &_sgp.uniforms[_sgp.cur_uniform-1];
    } else {
        return NULL;
    }
}

static sgp_uniform* _sgp_next_uniform() {
    if(SOKOL_LIKELY(_sgp.cur_uniform < _sgp.num_uniforms)) {
        return &_sgp.uniforms[_sgp.cur_uniform++];
    } else {
        _sgp_set_error(SGP_ERROR_UNIFORMS_FULL, "SGP uniform buffer is full");
        return NULL;
    }
}

static _sgp_command* _sgp_prev_command(uint32_t count) {
    if(SOKOL_LIKELY((_sgp.cur_command - _sgp.state._base_command) >= count)) {
        return &_sgp.commands[_sgp.cur_command-count];
    } else {
        return NULL;
    }
}

static _sgp_command* _sgp_next_command() {
    if(SOKOL_LIKELY(_sgp.cur_command < _sgp.num_commands)) {
        return &_sgp.commands[_sgp.cur_command++];
    } else {
        _sgp_set_error(SGP_ERROR_COMMANDS_FULL, "SGP command buffer is full");
        return NULL;
    }
}

void sgp_viewport(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same viewport
    if(_sgp.state.viewport.x == x && _sgp.state.viewport.y == y &&
       _sgp.state.viewport.w == w && _sgp.state.viewport.h == h)
        return;

    // try to reuse last command otherwise use the next one
    _sgp_command* cmd = _sgp_prev_command(1);
    if(!cmd || cmd->cmd != SGP_COMMAND_VIEWPORT)
        cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    *cmd = (_sgp_command) {
        .cmd = SGP_COMMAND_VIEWPORT,
        .args = {.viewport = {x, y, w, h}},
    };

    // adjust current scissor relative offset
    if(!(_sgp.state.scissor.w < 0 && _sgp.state.scissor.h < 0)) {
        _sgp.state.scissor.x += x - _sgp.state.viewport.x;
        _sgp.state.scissor.y += y - _sgp.state.viewport.y;
    }

    _sgp.state.viewport = (sgp_irect){x, y, w, h};
    _sgp.state.proj = _sgp_default_proj(w, h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_viewport() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_viewport(0, 0, _sgp.state.frame_size.w, _sgp.state.frame_size.h);
}

void sgp_scissor(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same scissor
    if(_sgp.state.scissor.x == x && _sgp.state.scissor.y == y &&
       _sgp.state.scissor.w == w && _sgp.state.scissor.h == h)
        return;

    // try to reuse last command otherwise use the next one
    _sgp_command* cmd = _sgp_prev_command(1);
    if(!cmd || cmd->cmd != SGP_COMMAND_SCISSOR)
        cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    // coordinate scissor in viewport subspace
    sgp_irect viewport_scissor = {_sgp.state.viewport.x + x, _sgp.state.viewport.y + y, w, h};

    // reset scissor
    if(w < 0 && h  < 0)
        viewport_scissor = (sgp_irect){0, 0, _sgp.state.frame_size.w, _sgp.state.frame_size.h};

    *cmd = (_sgp_command) {
        .cmd = SGP_COMMAND_SCISSOR,
        .args = {.scissor = viewport_scissor},
    };
    _sgp.state.scissor = (sgp_irect){x, y, w, h};
}

void sgp_reset_scissor() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_scissor(0, 0, -1, -1);
}

void sgp_reset_state() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_reset_viewport();
    sgp_reset_scissor();
    sgp_reset_proj();
    sgp_reset_transform();
    sgp_reset_blend_mode();
    sgp_reset_color();
}

static inline bool _sgp_uniform_equals(sgp_uniform* a, sgp_uniform* b) {
    return memcmp(a, b, sizeof(sgp_uniform)) == 0;
}

static inline bool _sgp_region_overlaps(_sgp_region a, _sgp_region b) {
    return !(a.x2 <= b.x1 || b.x2 <= a.x1  || a.y2 <= b.y1 || b.y2 <= a.y1);
}

static bool _sgp_merge_batch_command(sg_pipeline pip, sg_image img, sgp_uniform uniform, _sgp_region region, uint32_t vertex_index, uint32_t num_vertices) {
    _sgp_command* inter_cmds[SGP_BATCH_OPTIMIZER_DEPTH];
    _sgp_command* prev_cmd = NULL;
    uint32_t inter_cmd_count = 0;

    // find a command that is a good candidate to batch
    uint32_t lookup_depth = SGP_BATCH_OPTIMIZER_DEPTH;
    for(uint32_t depth=0;depth<lookup_depth;++depth) {
        _sgp_command* cmd = _sgp_prev_command(depth+1);
        // stop on nonexistent command
        if(!cmd)
            break;

        // command was optimized away, search deeper
        if(cmd->cmd == SGP_COMMAND_NONE) {
            lookup_depth++;
            continue;
        }

        // stop on scissor/viewport
        if(cmd->cmd != SGP_COMMAND_DRAW)
            break;

        // can only batch commands with the same bindings and uniforms
        if(cmd->args.draw.pip.id == pip.id &&
           cmd->args.draw.img.id == img.id &&
           _sgp_uniform_equals(&uniform, &_sgp.uniforms[cmd->args.draw.uniform_index])) {
            prev_cmd = cmd;
            break;
        } else {
            inter_cmds[inter_cmd_count] = cmd;
            inter_cmd_count++;
        }
    }

    if(!prev_cmd)
        return false;

    // allow batching only if the region of the current or previous draw
    // is not touched by intermediate commands
    bool overlaps_next = false;
    bool overlaps_prev = false;
    _sgp_region prev_region = prev_cmd->args.draw.region;
    for(uint32_t i=0;i<inter_cmd_count;++i) {
        _sgp_region inter_region = inter_cmds[i]->args.draw.region;
        if(_sgp_region_overlaps(region, inter_region)) {
            overlaps_next = true;
            if(overlaps_prev) return false;
        }
        if(_sgp_region_overlaps(prev_region, inter_region)) {
            overlaps_prev = true;
            if(overlaps_next) return false;
        }
    }

    if(!overlaps_next) { // batch in the previous draw command
        if(inter_cmd_count > 0) {
            // not enough vertices space, can't do this batch
            if(SOKOL_UNLIKELY(_sgp.cur_vertex + num_vertices > _sgp.num_vertices))
                return false;

            // rearrange vertices memory for the batch
            uint32_t prev_end_vertex = prev_cmd->args.draw.vertex_index + prev_cmd->args.draw.num_vertices;
            uint32_t move_count = _sgp.cur_vertex - prev_end_vertex;
            memmove(&_sgp.vertices[prev_end_vertex + num_vertices], &_sgp.vertices[prev_end_vertex], move_count * sizeof(_sgp_vertex));
            memcpy(&_sgp.vertices[prev_end_vertex], &_sgp.vertices[vertex_index + num_vertices], num_vertices * sizeof(_sgp_vertex));

            // offset vertices of intermediate draw commands
            for(uint32_t i=0;i<inter_cmd_count;++i)
                inter_cmds[i]->args.draw.vertex_index += num_vertices;
        }

        // update draw region and vertices
        prev_region.x1 = _sg_min(prev_region.x1, region.x1);
        prev_region.y1 = _sg_min(prev_region.y1, region.y1);
        prev_region.x2 = _sg_max(prev_region.x2, region.x2);
        prev_region.y2 = _sg_max(prev_region.y2, region.y2);
        prev_cmd->args.draw.num_vertices += num_vertices;
        prev_cmd->args.draw.region = prev_region;
    } else { // batch in the next draw command
        SOKOL_ASSERT(inter_cmd_count > 0);

        // append new draw command
        _sgp_command* cmd = _sgp_next_command();
        if(SOKOL_UNLIKELY(!cmd))
            return false;

        uint32_t prev_num_vertices = prev_cmd->args.draw.num_vertices;
        // not enough vertices space, can't do this batch
        if(SOKOL_UNLIKELY(_sgp.cur_vertex + prev_num_vertices > _sgp.num_vertices))
            return false;

        // rearrange vertices memory for the batch
        memmove(&_sgp.vertices[vertex_index + prev_num_vertices], &_sgp.vertices[vertex_index], num_vertices * sizeof(_sgp_vertex));
        memcpy(&_sgp.vertices[vertex_index], &_sgp.vertices[prev_cmd->args.draw.vertex_index], prev_num_vertices * sizeof(_sgp_vertex));

        // update draw region and vertices
        prev_region.x1 = _sg_min(prev_region.x1, region.x1);
        prev_region.y1 = _sg_min(prev_region.y1, region.y1);
        prev_region.x2 = _sg_max(prev_region.x2, region.x2);
        prev_region.y2 = _sg_max(prev_region.y2, region.y2);
        _sgp.cur_vertex += prev_num_vertices;
        num_vertices += prev_num_vertices;

        // configure the draw command
        cmd->cmd = SGP_COMMAND_DRAW,
        cmd->args.draw = (_sgp_draw_args){
            .pip = pip,
            .img = img,
            .region = prev_region,
            .uniform_index = prev_cmd->args.draw.uniform_index,
            .vertex_index = vertex_index,
            .num_vertices = num_vertices,
        };

        // force skipping the previous draw command
        prev_cmd->cmd = SGP_COMMAND_NONE;
    }
    return true;
}

static void _sgp_queue_draw(sg_pipeline pip, sg_image img, _sgp_region region, uint32_t vertex_index, uint32_t num_vertices) {
    if(pip.id == SG_INVALID_ID)
        return;

    // try to merge on previous command to draw in a batch
    if(_sgp_merge_batch_command(pip, img, _sgp.state.uniform, region, vertex_index, num_vertices))
        return;

    // setup uniform, try to reuse previous uniform when possible
    sgp_uniform *prev_uniform = _sgp_prev_uniform();
    bool reuse_uniform = prev_uniform && _sgp_uniform_equals(prev_uniform, &_sgp.state.uniform);
    if(!reuse_uniform) {
        // append new uniform
        sgp_uniform *uniform = _sgp_next_uniform();
        if(SOKOL_UNLIKELY(!uniform)) return;
        *uniform = _sgp.state.uniform;
    }
    uint32_t uniform_index = _sgp.cur_uniform - 1;

    // append new draw command
    _sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;
    cmd->cmd = SGP_COMMAND_DRAW,
    cmd->args.draw = (_sgp_draw_args){
        .pip = pip,
        .img = img,
        .region = region,
        .uniform_index = uniform_index,
        .vertex_index = vertex_index,
        .num_vertices = num_vertices,
    };
}

static inline sgp_vec2 _sgp_mat3_vec2_mul(const sgp_mat3* m, const sgp_vec2* v) {
    return (sgp_vec2){
        .x = m->v[0][0]*v->x + m->v[0][1]*v->y + m->v[0][2],
        .y = m->v[1][0]*v->x + m->v[1][1]*v->y + m->v[1][2]
    };
}

static void _sgp_transform_vec2(sgp_mat3* matrix, sgp_vec2* dst, const sgp_vec2 *src, uint32_t count) {
    for(uint32_t i=0;i<count;++i)
        dst[i] = _sgp_mat3_vec2_mul(matrix, &src[i]);
}

static void _sgp_draw_solid_pip(sg_pipeline pip, const sgp_vec2* vertices, uint32_t num_vertices) {
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* transformed_vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {.x1=1.0f, .y1=1.0f, .x2=-1.0f, .y2=-1.0f};
    for(uint32_t i=0;i<num_vertices;++i) {
        sgp_vec2 p = _sgp_mat3_vec2_mul(&mvp, &vertices[i]);
        region.x1 = _sg_min(region.x1, p.x);
        region.y1 = _sg_min(region.y1, p.y);
        region.x2 = _sg_max(region.x2, p.x);
        region.y2 = _sg_max(region.y2, p.y);
        transformed_vertices[i] = (_sgp_vertex) {
            .position = p,
            .texcoord = {0.0f, 0.0f}
        };
    }
    _sgp_queue_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
}

void sgp_clear() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // setup vertices
    uint32_t num_vertices = 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_vec2 quad[4] = {
        {-1.0f, -1.0f}, // bottom left
        { 1.0f, -1.0f}, // bottom right
        { 1.0f,  1.0f}, // top right
        {-1.0f,  1.0f}, // top left
    };
    const sgp_vec2 texcoord = {0.0f, 0.0f};

    // make a quad composed of 2 triangles
    v[0].position = quad[0]; v[0].texcoord = texcoord;
    v[1].position = quad[1]; v[1].texcoord = texcoord;
    v[2].position = quad[2]; v[2].texcoord = texcoord;
    v[3].position = quad[3]; v[3].texcoord = texcoord;
    v[4].position = quad[0]; v[4].texcoord = texcoord;
    v[5].position = quad[2]; v[5].texcoord = texcoord;

    _sgp_region region = {.x1=-1.0f, .y1=-1.0f, .x2=1.0f, .y2=1.0f};

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_queue_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
}

void sgp_draw_points(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_point(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_point point = {x, y};
    sgp_draw_points(&point, 1);
}

void sgp_draw_lines(const sgp_line* lines, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)lines, count*2);
}

void sgp_draw_line(float ax, float ay, float bx, float by) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_line line = {{ax,ay},{bx, by}};
    sgp_draw_lines(&line, 1);
}

void sgp_draw_line_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)triangles, count*3);
}

void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_triangle triangle = {{ax,ay},{bx, by},{cx, cy}};
    sgp_draw_filled_triangles(&triangle, 1);
}

void sgp_draw_filled_triangle_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_rect* rect = rects;
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {.x1=1.0f, .y1=1.0f, .x2=-1.0f, .y2=-1.0f};
    for(uint32_t i=0;i<count;v+=6, rect++, i++) {
        sgp_vec2 quad[4] = {
            {rect->x,           rect->y + rect->h}, // bottom left
            {rect->x + rect->w, rect->y + rect->h}, // bottom right
            {rect->x + rect->w, rect->y}, // top right
            {rect->x,  rect->y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t i=0;i<4;++i) {
            region.x1 = _sg_min(region.x1, quad[i].x);
            region.y1 = _sg_min(region.y1, quad[i].y);
            region.x2 = _sg_max(region.x2, quad[i].x);
            region.y2 = _sg_max(region.y2, quad[i].y);
        }

        const sgp_vec2 texcoord = {0.0f, 0.0f};

        // make a quad composed of 2 triangles
        v[0].position = quad[0]; v[0].texcoord = texcoord;
        v[1].position = quad[1]; v[1].texcoord = texcoord;
        v[2].position = quad[2]; v[2].texcoord = texcoord;
        v[3].position = quad[3]; v[3].texcoord = texcoord;
        v[4].position = quad[0]; v[4].texcoord = texcoord;
        v[5].position = quad[2]; v[5].texcoord = texcoord;
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
}

void sgp_draw_filled_rect(float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x,y,w,h};
    sgp_draw_filled_rects(&rect, 1);
}

void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {.x1=1.0f, .y1=1.0f, .x2=-1.0f, .y2=-1.0f};
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].x,              rects[i].y + rects[i].h}, // bottom left
            {rects[i].x + rects[i].w, rects[i].y + rects[i].h}, // bottom right
            {rects[i].x + rects[i].w, rects[i].y}, // top right
            {rects[i].x,  rects[i].y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t i=0;i<4;++i) {
            region.x1 = _sg_min(region.x1, quad[i].x);
            region.y1 = _sg_min(region.y1, quad[i].y);
            region.x2 = _sg_max(region.x2, quad[i].x);
            region.y2 = _sg_max(region.y2, quad[i].y);
        }

        const sgp_vec2 vtexquad[4] = {
            {0.0f, 1.0f}, // bottom left
            {1.0f, 1.0f}, // bottom right
            {1.0f, 0.0f}, // top right
            {0.0f, 0.0f}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0]; v[0].texcoord = vtexquad[0];
        v[1].position = quad[1]; v[1].texcoord = vtexquad[1];
        v[2].position = quad[2]; v[2].texcoord = vtexquad[2];
        v[3].position = quad[3]; v[3].texcoord = vtexquad[3];
        v[4].position = quad[0]; v[4].texcoord = vtexquad[0];
        v[5].position = quad[2]; v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, image, region, vertex_index, num_vertices);
}

void sgp_draw_textured_rect(sg_image image, float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x, y, w, h};
    sgp_draw_textured_rects(image, &rect, 1);
}

static sgp_isize _sgp_query_image_size(sg_image img_id) {
    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    return (sgp_isize){img->cmn.width, img->cmn.height};
}

void sgp_draw_textured_rects_ex(sg_image image, const sgp_textured_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute image values used for texture coords transform
    sgp_isize image_size = _sgp_query_image_size(image);
    if(SOKOL_UNLIKELY(image_size.w == 0 || image_size.h == 0)) return;
    float iw = 1.0f/image_size.w, ih = 1.0f/image_size.h;

    // compute vertices
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {.x1=1.0f, .y1=1.0f, .x2=-1.0f, .y2=-1.0f};
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].dst.x,                   rects[i].dst.y + rects[i].dst.h}, // bottom left
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y + rects[i].dst.h}, // bottom right
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y}, // top right
            {rects[i].dst.x,  rects[i].dst.y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t i=0;i<4;++i) {
            region.x1 = _sg_min(region.x1, quad[i].x);
            region.y1 = _sg_min(region.y1, quad[i].y);
            region.x2 = _sg_max(region.x2, quad[i].x);
            region.y2 = _sg_max(region.y2, quad[i].y);
        }

        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0];
        v[1].position = quad[1];
        v[2].position = quad[2];
        v[3].position = quad[3];
        v[4].position = quad[0];
        v[5].position = quad[2];
    }

    // compute texture coords
    for(uint32_t i=0;i<count;i++) {
        // compute source rect
        float tl = rects[i].src.x*iw;
        float tt = rects[i].src.y*ih;
        float tr = (rects[i].src.x + rects[i].src.w)*iw;
        float tb = (rects[i].src.y + rects[i].src.h)*ih;
        sgp_vec2 vtexquad[4] = {
            {tl, tb}, // bottom left
            {tr, tb}, // bottom right
            {tr, tt}, // top right
            {tl, tt}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].texcoord = vtexquad[0];
        v[1].texcoord = vtexquad[1];
        v[2].texcoord = vtexquad[2];
        v[3].texcoord = vtexquad[3];
        v[4].texcoord = vtexquad[0];
        v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, image, region, vertex_index, num_vertices);
}

void sgp_draw_textured_rect_ex(sg_image image, sgp_rect dest_rect, sgp_rect src_rect) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_textured_rect rect = {dest_rect, src_rect};
    sgp_draw_textured_rects_ex(image, &rect, 1);
}

sgp_desc sgp_query_desc() {
    return _sgp.desc;
}

sgp_state* sgp_query_state() {
    return &_sgp.state;
}

#endif // SOKOL_GP_IMPL_INCLUDED
#endif // SOKOL_GP_IMPL
