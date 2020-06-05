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
    SGP_ERROR_COMMAND_BUFFERS_FULL,
    SGP_ERROR_TRANSFORM_STACK_OVERFLOW,
    SGP_ERROR_TRANSFORM_STACK_UNDERFLOW,
    SGP_ERROR_STATE_STACK_OVERFLOW,
    SGP_ERROR_STATE_STACK_UNDERFLOW
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
    uint32_t max_vertices;
    uint32_t max_commands;
    void (*swap_cb)();
    void (*activate_cb)(bool);
    void (*pump_events)();
} sgp_desc;

typedef struct sgp_uniform {
    sgp_color color;
} sgp_uniform;

typedef struct sgp_pipeline {
    uint32_t id;
} sgp_pipeline;

typedef struct _sgp_command_buffer _sgp_command_buffer;

typedef struct sgp_state {
    sgp_isize frame_size;
    sgp_irect viewport;
    sgp_irect scissor;
    sgp_mat3 proj;
    sgp_mat3 transform;
    sgp_mat3 mvp;
    sgp_uniform uniform;
    sgp_blend_mode blend_mode;
    _sgp_command_buffer *cbuf;
} sgp_state;

// setup functions
SOKOL_API_DECL bool sgp_setup(const sgp_desc* desc);
SOKOL_API_DECL void sgp_shutdown();
SOKOL_API_DECL bool sgp_is_valid();
SOKOL_API_DECL sgp_error sgp_get_error_code();
SOKOL_API_DECL const char* sgp_get_error();

// rendering functions
SOKOL_API_DECL void sgp_begin_default_pass(const sg_pass_action* pass_action, int width, int height);
SOKOL_API_DECL void sgp_begin_pass(sg_pass pass_id, const sg_pass_action* pass_action);
SOKOL_API_DECL void sgp_end_pass();

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
//SOKOL_API_DECL void sgp_set_pipeline(sg_pipeline pip);
//SOKOL_API_DECL void sgp_reset_pipeline();
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

// multi thread
SOKOL_API_DECL bool sgp_set_multithread(bool enable);
SOKOL_API_DECL bool sgp_is_multithread();
SOKOL_API_DECL void sgp_synchronize();
SOKOL_API_DECL void sgp_lock();
SOKOL_API_DECL void sgp_unlock();

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
    _SGP_MAX_COMMAND_BUFFERS = 8,
    _SGP_TRANSFORM_STACK_SIZE = 64,
    _SGP_MULTITHREAD_QUEUE_SIZE = 3
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

typedef struct _sgp_command_buffer {
    uint32_t cur_vertex;
    uint32_t cur_uniform;
    uint32_t cur_command;
    bool in_use;
    sgp_isize pass_size;
    sg_pass pass;
    sg_pass_action pass_action;
    _sgp_vertex* vertices;
    sgp_uniform* uniforms;
    _sgp_command* commands;
} _sgp_command_buffer;

typedef struct _sgp_context {
    uint32_t init_cookie;
    const char *last_error;
    sgp_error last_error_code;
    sgp_desc desc;

    // resources
    sg_shader shader;
    sg_buffer vertex_buf;
    sg_image white_img;
    sg_pipeline pipelines[_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM];
    _sgp_command_buffer cbufs[_SGP_MAX_COMMAND_BUFFERS];

    // multi thread
    bool multithread;
    bool working;
    bool worker_context_active;
    bool worker_idle;
    bool worker_running;
    bool locked;
    thrd_t worker;
    _sgp_command_buffer* cqueue[_SGP_MULTITHREAD_QUEUE_SIZE];
    mtx_t worker_mtx;
    cnd_t wake_worker_cnd;
    cnd_t wake_main_cnd;

    // state tracking
    sgp_state state;

    // matrix stack
    uint32_t cur_transform;
    uint32_t cur_state;
    sgp_mat3 transform_stack[_SGP_TRANSFORM_STACK_SIZE];
    sgp_state state_stack[_SGP_MAX_COMMAND_BUFFERS];
} _sgp_context;

static _sgp_context _sgp;

static const sgp_mat3 _sgp_mat3_identity = {{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
}};

static void _sgp_set_error(sgp_error error_code, const char *error) {
    _sgp.last_error_code = error_code;
    _sgp.last_error = error;
    SOKOL_LOG(error);
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

#elif defined(SOKOL_DUMMY_BACKEND)
static const char _sgp_vs_source[] = "";
static const char _sgp_fs_source[] = "";
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
    blend.depth_format = SG_PIXELFORMAT_NONE;

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
    SOKOL_ASSERT(pip.id != SG_INVALID_ID);
    _sgp.pipelines[pip_index] = pip;
    return pip;
};

static void _sgp_setup_pipelines() {
    // create shaders
    sg_shader_desc shader_desc = {
        .attrs = {
            {.name="coord", .sem_name="POSITION", .sem_index=0},
        },
        .vs = {.source = _sgp_vs_source},
        .fs = {
            .source = _sgp_fs_source,
            .uniform_blocks = {{
                .size = 4*sizeof(float),
                .uniforms = {{.name="color", .type=SG_UNIFORMTYPE_FLOAT4}},
            }},
            .images = {{.name = "tex", .type=SG_IMAGETYPE_2D}},
        },
    };
    _sgp.shader = sg_make_shader(&shader_desc);
    SOKOL_ASSERT(_sgp.shader.id != SG_INVALID_ID);

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
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_BLEND);
}

static _sgp_command_buffer* _sgp_acquire_command_buffer() {
    // lock because `in_use` field can be read/write in different threads
    if(_sgp.multithread) {
        int res = mtx_lock(&_sgp.worker_mtx);
        SOKOL_ASSERT(res == thrd_success); _SOKOL_UNUSED(res);
    }

    for(uint32_t i=0;i<_SGP_MAX_COMMAND_BUFFERS;++i) {
        _sgp_command_buffer *cbuf = &_sgp.cbufs[i];
        if(!cbuf->in_use) {
            cbuf->in_use = true;

            // found a available command buffer, return
            if(_sgp.multithread) {
                int res = mtx_unlock(&_sgp.worker_mtx);
                SOKOL_ASSERT(res == thrd_success); _SOKOL_UNUSED(res);
            }
            return cbuf;
        }
    }

    // this should never happens, we always expects an available command buffer
    SOKOL_UNREACHABLE;
    return NULL;
}

static void _sgp_release_command_buffer(_sgp_command_buffer* cbuf) {
    SOKOL_ASSERT(cbuf);
    // this function expects that `worker_mtx` is already locked

    // reset command buffer to be in_use again
    cbuf->in_use = false;
    cbuf->cur_vertex = 0;
    cbuf->cur_uniform = 0;
    cbuf->cur_command = 0;
}

static void _sgp_cnd_wait_pumped(cnd_t* cnd, mtx_t* mtx) {
    int res;
    _SOKOL_UNUSED(res);

    // wait until there is a free queue slot
    if(_sgp.desc.pump_events) {
        // wait indefinitely for worker a little and pump events on timeout
        struct timespec until = {0, 0};

        res = timespec_get(&until, TIME_UTC);
        SOKOL_ASSERT(res == TIME_UTC);

        // mark to wait until 4 milliseconds in the future
        until.tv_nsec += 4 * 1000000; // 4 ms
        until.tv_sec += until.tv_nsec / 1000000000;
        until.tv_nsec = until.tv_nsec % 1000000000;

        // wait for signal for a few milliseconds
        res = cnd_timedwait(cnd, mtx, &until);
        SOKOL_ASSERT(res != thrd_error);

        // pump events on timeouts
        if(res == thrd_timedout)
            _sgp.desc.pump_events();
    } else {
        // wait indefinitely for worker
        res = cnd_wait(cnd, mtx);
        SOKOL_ASSERT(res == thrd_success);
    }
}

static void _sgp_enqueue_command_buffer(_sgp_command_buffer* cbuf) {
    int res;
    _SOKOL_UNUSED(res);

    // lock
    res = mtx_lock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    while(true) {
        for(int i=0;i<_SGP_MULTITHREAD_QUEUE_SIZE;++i) {
            if(_sgp.cqueue[i] == NULL) {
                // found an available queue slot, use it
                _sgp.cqueue[i] = cbuf;

                // notify the worker that we have a new queued job
                res = cnd_signal(&_sgp.wake_worker_cnd);
                SOKOL_ASSERT(res == thrd_success);

                // can unlock now
                res = mtx_unlock(&_sgp.worker_mtx);
                SOKOL_ASSERT(res == thrd_success);
                return;
            }
        }

        _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);
    }

    // this should never happen
    SOKOL_UNREACHABLE;
}

bool sgp_setup(const sgp_desc* desc) {
    SOKOL_ASSERT(_sgp.init_cookie == 0);

    // init
    _sgp.init_cookie = _SGP_INIT_COOKIE;
    _sgp.last_error = "";

    // set desc default values
    _sgp.desc = *desc;
    _sgp.desc.max_vertices = _sg_def(desc->max_vertices, (uint32_t)_SGP_DEFAULT_MAX_VERTICES);
    _sgp.desc.max_commands = _sg_def(desc->max_commands, (uint32_t)_SGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    for(uint32_t i=0;i<_SGP_MAX_COMMAND_BUFFERS;++i) {
        _sgp_command_buffer *cbuf = &_sgp.cbufs[i];
        cbuf->vertices = (_sgp_vertex*) SOKOL_MALLOC(_sgp.desc.max_vertices * sizeof(_sgp_vertex));
        cbuf->uniforms = (sgp_uniform*) SOKOL_MALLOC(_sgp.desc.max_commands * sizeof(sgp_uniform));
        cbuf->commands = (_sgp_command*) SOKOL_MALLOC(_sgp.desc.max_commands * sizeof(_sgp_command));
        SOKOL_ASSERT(cbuf->commands && cbuf->uniforms && cbuf->uniforms);
        memset(cbuf->vertices, 0, _sgp.desc.max_vertices * sizeof(_sgp_vertex));
        memset(cbuf->uniforms, 0, _sgp.desc.max_commands * sizeof(sgp_uniform));
        memset(cbuf->commands, 0, _sgp.desc.max_commands * sizeof(_sgp_command));
    }

    // create vertex buffer
    sg_buffer_desc vertex_buf_desc = {
        .size = (int)(_sgp.desc.max_vertices * sizeof(_sgp_vertex)),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
    };
    _sgp.vertex_buf = sg_make_buffer(&vertex_buf_desc);
    SOKOL_ASSERT(_sgp.vertex_buf.id != SG_INVALID_ID);

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
    SOKOL_ASSERT(_sgp.white_img.id != SG_INVALID_ID);

    _sgp_setup_pipelines();
    return true;
}

void sgp_shutdown() {
    if(_sgp.init_cookie == 0) return; // not initialized

    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state == 0);

    // wait worker thread to finish it jobs first
    sgp_set_multithread(false);

    // free all command bufers
    for(uint32_t i=0;i<_SGP_MAX_COMMAND_BUFFERS;++i) {
        _sgp_command_buffer *cbuf = &_sgp.cbufs[i];
        SOKOL_FREE(cbuf->vertices);
        SOKOL_FREE(cbuf->uniforms);
        SOKOL_FREE(cbuf->commands);
    }

    // destroy all allocate sokol stuff
    for(uint32_t i=0;i<_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM;++i) {
        sg_pipeline pip = _sgp.pipelines[i];
        if(pip.id != SG_INVALID_ID)
            sg_destroy_pipeline(pip);
    }
    sg_destroy_shader(_sgp.shader);
    sg_destroy_buffer(_sgp.vertex_buf);
    sg_destroy_image(_sgp.white_img);


    _sgp = (_sgp_context){.init_cookie=0};
}

bool sgp_is_valid() {
    return _sgp.init_cookie == _SGP_INIT_COOKIE;
}

sgp_error sgp_get_error_code() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    return _sgp.last_error_code;
}

const char* sgp_get_error() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    return _sgp.last_error;
}

static inline sgp_mat3 _sgp_default_proj(int width, int height) {
    // matrix to convert screen coordinate system
    // to the usual the coordinate system in_use on the backends
    return (sgp_mat3){{
        {2.0f/width,           0.0f, -1.0f},
        {      0.0f,   -2.0f/height,  1.0f},
        {      0.0f,           0.0f,  1.0f}
    }};
}

sg_pass_action _sgp_resolve_default_pass_action(const sg_pass_action* from) {
    sg_pass_action to = *from;
    for(int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        if(to.colors[i].action  == _SG_ACTION_DEFAULT) {
            to.colors[i].action = SG_ACTION_CLEAR;
            to.colors[i].val[0] = 0.0f;
            to.colors[i].val[1] = 0.0f;
            to.colors[i].val[2] = 0.0f;
            to.colors[i].val[3] = 0.0f;
        }
    }
    if(to.depth.action == _SG_ACTION_DEFAULT)
        to.depth.action = SG_ACTION_DONTCARE;
    if(to.stencil.action == _SG_ACTION_DEFAULT)
        to.depth.action = SG_ACTION_DONTCARE;
    return to;
}

static void _sgp_push_state(sg_pass pass, const sg_pass_action* pass_action, int width, int height) {
    if(SOKOL_UNLIKELY(_sgp.cur_state >= _SGP_MAX_COMMAND_BUFFERS)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_OVERFLOW, "SGP state stack overflow");
        return;
    }

    // first begin, reset last error
    if(_sgp.cur_state == 0) {
        _sgp.last_error = "";
        _sgp.last_error_code = SGP_NO_ERROR;
    }

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
    _sgp.state.cbuf = _sgp_acquire_command_buffer();
    _sgp.state.cbuf->pass = pass;
    _sgp.state.cbuf->pass_action = _sgp_resolve_default_pass_action(pass_action);
    _sgp.state.cbuf->pass_size = _sgp.state.frame_size;
}

static void _sgp_pop_state() {
    if(SOKOL_UNLIKELY(_sgp.cur_state <= 0)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_UNDERFLOW, "SGP state stack underflow");
        return;
    }

    // restore old state
    _sgp.state = _sgp.state_stack[--_sgp.cur_state];
}

void sgp_begin_default_pass(const sg_pass_action* pass_action, int width, int height) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp_push_state((sg_pass){0}, pass_action, width, height);
}

void sgp_begin_pass(sg_pass pass, const sg_pass_action* pass_action) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sg_pass_t* _pass = _sg_lookup_pass(&_sg.pools, pass.id);
    _sg_image_t* _img = _sg_pass_color_image(_pass, 0);
    SOKOL_ASSERT(_img);
    _sgp_push_state(pass, pass_action, _img->cmn.width, _img->cmn.height);
}

static void _sgp_flush(_sgp_command_buffer* cbuf) {
    if(cbuf->cur_command <= 0)
        return;

    // define the resource bindings
    sg_bindings bind = (sg_bindings){
        .vertex_buffers = {_sgp.vertex_buf},
    };

    // flush commands
    const uint32_t SG_IMPOSSIBLE_ID = 0xffffffffU;
    uint32_t cur_pip_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_img_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_uniform_index = SG_IMPOSSIBLE_ID;
    sg_update_buffer(_sgp.vertex_buf, cbuf->vertices, cbuf->cur_vertex * sizeof(_sgp_vertex));
    for(uint32_t i = 0; i < cbuf->cur_command; ++i) {
        _sgp_command* cmd = &cbuf->commands[i];
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
                    cur_img_id = args->img.id;
                    bind.fs_images[0] = args->img;
                    sg_apply_bindings(&bind);
                }
                if(cur_uniform_index != args->uniform_index) {
                    cur_uniform_index = args->uniform_index;
                    sgp_uniform* uniform = &cbuf->uniforms[cur_uniform_index];
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, uniform, sizeof(sgp_uniform));
                }
                sg_draw(args->vertex_index, args->num_vertices, 1);
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
}

static void _sgp_run_command_buffer(_sgp_command_buffer* cbuf) {
    if(cbuf->pass.id == SG_INVALID_ID) {
        // begin default pass (usually the back buffer)
        sg_begin_default_pass(&cbuf->pass_action, cbuf->pass_size.w, cbuf->pass_size.h);
    } else {
        // being a custom pass (usually frame buffers)
        sg_begin_pass(cbuf->pass, &cbuf->pass_action);
    }

    // flush all drawing commands
    _sgp_flush(cbuf);

    // end pass
    sg_end_pass();

    // commit all drawing commands
    sg_commit();
}

void sgp_end_pass() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp_command_buffer *cbuf = _sgp.state.cbuf;
    SOKOL_ASSERT(cbuf);

    // only execute the drawing commands if no errors were found
    if(_sgp.last_error_code == SGP_NO_ERROR) {
        if(_sgp.multithread) {
            // drawing commands will be executed in the worker thread
            _sgp_enqueue_command_buffer(cbuf);
        } else {
            // execute drawing commands in this thread
            _sgp_run_command_buffer(cbuf);
        }
    }

    if(!_sgp.multithread) {
        // swap buffers in case of default pass and swap_cb is set
        if(cbuf->pass.id == 0 && _sgp.desc.swap_cb)
            _sgp.desc.swap_cb();

        // this command buffer can now be reused
        _sgp_release_command_buffer(cbuf);
    }

    // restore old state so we can continue increasing old command buffer
    _sgp_pop_state();
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
    if(SOKOL_UNLIKELY(_sgp.cur_transform >= _SGP_TRANSFORM_STACK_SIZE)) {
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
    if(SOKOL_LIKELY(_sgp.state.cbuf->cur_vertex + count <= _sgp.desc.max_vertices)) {
        _sgp_vertex *vertices = &_sgp.state.cbuf->vertices[_sgp.state.cbuf->cur_vertex];
        _sgp.state.cbuf->cur_vertex += count;
        return vertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL, "SGP vertices buffer is full");
        return NULL;
    }
}

static sgp_uniform* _sgp_prev_uniform() {
    if(SOKOL_LIKELY(_sgp.state.cbuf->cur_uniform > 0)) {
        return &_sgp.state.cbuf->uniforms[_sgp.state.cbuf->cur_uniform-1];
    } else {
        return NULL;
    }
}

static sgp_uniform* _sgp_next_uniform() {
    if(SOKOL_LIKELY(_sgp.state.cbuf->cur_uniform < _sgp.desc.max_commands)) {
        return &_sgp.state.cbuf->uniforms[_sgp.state.cbuf->cur_uniform++];
    } else {
        _sgp_set_error(SGP_ERROR_UNIFORMS_FULL, "SGP uniform buffer is full");
        return NULL;
    }
}

static _sgp_command* _sgp_prev_command(uint32_t count) {
    if(SOKOL_LIKELY(_sgp.state.cbuf->cur_command >= count)) {
        return &_sgp.state.cbuf->commands[_sgp.state.cbuf->cur_command-count];
    } else {
        return NULL;
    }
}

static _sgp_command* _sgp_next_command() {
    if(SOKOL_LIKELY(_sgp.state.cbuf->cur_command < _sgp.desc.max_commands)) {
        return &_sgp.state.cbuf->commands[_sgp.state.cbuf->cur_command++];
    } else {
        _sgp_set_error(SGP_ERROR_COMMANDS_FULL, "SGP command list is full");
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

    _sgp_command* cmd = _sgp_next_command();
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

    _sgp_command* cmd = _sgp_next_command();
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
           _sgp_uniform_equals(&uniform, &_sgp.state.cbuf->uniforms[cmd->args.draw.uniform_index])) {
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
            if(SOKOL_UNLIKELY(_sgp.state.cbuf->cur_vertex + num_vertices > _sgp.desc.max_vertices))
                return false;

            // rearrange vertices memory for the batch
            uint32_t prev_end_vertex = prev_cmd->args.draw.vertex_index + prev_cmd->args.draw.num_vertices;
            uint32_t move_count = _sgp.state.cbuf->cur_vertex - prev_end_vertex;
            memmove(&_sgp.state.cbuf->vertices[prev_end_vertex + num_vertices], &_sgp.state.cbuf->vertices[prev_end_vertex], move_count * sizeof(_sgp_vertex));
            memcpy(&_sgp.state.cbuf->vertices[prev_end_vertex], &_sgp.state.cbuf->vertices[vertex_index + num_vertices], num_vertices * sizeof(_sgp_vertex));

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
        if(SOKOL_UNLIKELY(_sgp.state.cbuf->cur_vertex + prev_num_vertices > _sgp.desc.max_vertices))
            return false;

        // rearrange vertices memory for the batch
        memmove(&_sgp.state.cbuf->vertices[vertex_index + prev_num_vertices], &_sgp.state.cbuf->vertices[vertex_index], num_vertices * sizeof(_sgp_vertex));
        memcpy(&_sgp.state.cbuf->vertices[vertex_index], &_sgp.state.cbuf->vertices[prev_cmd->args.draw.vertex_index], prev_num_vertices * sizeof(_sgp_vertex));

        // update draw region and vertices
        prev_region.x1 = _sg_min(prev_region.x1, region.x1);
        prev_region.y1 = _sg_min(prev_region.y1, region.y1);
        prev_region.x2 = _sg_max(prev_region.x2, region.x2);
        prev_region.y2 = _sg_max(prev_region.y2, region.y2);
        _sgp.state.cbuf->cur_vertex += prev_num_vertices;
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

static void _sgp_draw(sg_pipeline pip, sg_image img, _sgp_region region, uint32_t vertex_index, uint32_t num_vertices) {
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
    uint32_t uniform_index = _sgp.state.cbuf->cur_uniform - 1;

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
    uint32_t vertex_index = _sgp.state.cbuf->cur_vertex;
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
    _sgp_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
}

void sgp_clear() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // setup vertices
    uint32_t num_vertices = 6;
    uint32_t vertex_index = _sgp.state.cbuf->cur_vertex;
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
    _sgp_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
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
    uint32_t vertex_index = _sgp.state.cbuf->cur_vertex;
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
    _sgp_draw(pip, _sgp.white_img, region, vertex_index, num_vertices);
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
    uint32_t vertex_index = _sgp.state.cbuf->cur_vertex;
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
    _sgp_draw(pip, image, region, vertex_index, num_vertices);
}

void sgp_draw_textured_rect(sg_image image, float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x, y, w, h};
    sgp_draw_textured_rects(image, &rect, 1);
}

static sgp_isize _sgp_query_image_size(sg_image img) {
    const _sg_image_t* _img = _sg_lookup_image(&_sg.pools, img.id);
    return (sgp_isize){_img->cmn.width, _img->cmn.height};
}

void sgp_draw_textured_rects_ex(sg_image image, const sgp_textured_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.state.cbuf->cur_vertex;
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
    _sgp_draw(pip, image, region, vertex_index, num_vertices);
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

static int _sgp_worker_thread(void* param) {
    _SOKOL_UNUSED(param);
    int res;
    _SOKOL_UNUSED(res);

    // start locked
    res = mtx_lock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    // activate graphics context on this thread
    _sgp.worker_context_active = true;
    if(_sgp.desc.activate_cb)
        _sgp.desc.activate_cb(true);

    // notify that worker has started
    _sgp.worker_running = true;
    res = cnd_signal(&_sgp.wake_main_cnd);
    SOKOL_ASSERT(res == thrd_success);

    while(true) {
        // sgp_lock/sgp_unlock requested, switch graphics context from this thread
        if(_sgp.locked == _sgp.worker_context_active) {
            if(_sgp.desc.activate_cb)
                _sgp.desc.activate_cb(!_sgp.locked);
            _sgp.worker_context_active = !_sgp.locked;

            // signal that the context switch is done
            res = cnd_signal(&_sgp.wake_main_cnd);
            SOKOL_ASSERT(res == thrd_success);
        }
        // command buffer job is ready
        else if (_sgp.cqueue[0] != NULL) {
            SOKOL_ASSERT(_sgp.worker_context_active);

            // pop next command buffer from the queue
            _sgp_command_buffer* cbuf = _sgp.cqueue[0];
            SOKOL_ASSERT(cbuf);
            memmove(&_sgp.cqueue[0], &_sgp.cqueue[1], (_SGP_MULTITHREAD_QUEUE_SIZE - 1) * sizeof(_sgp_command_buffer*));
            _sgp.cqueue[_SGP_MULTITHREAD_QUEUE_SIZE - 1] = NULL;

            // notify that a queue slot was freed and a new command buffer can be queued
            res = cnd_signal(&_sgp.wake_main_cnd);
            SOKOL_ASSERT(res == thrd_success);

            // unlock
            res = mtx_unlock(&_sgp.worker_mtx);
            SOKOL_ASSERT(res == thrd_success);

            // execute the command buffer
            _sgp_run_command_buffer(cbuf);

            // do swap in case of default pass
            if(cbuf->pass.id == 0 && _sgp.desc.swap_cb)
                _sgp.desc.swap_cb();

            // lock again
            res = mtx_lock(&_sgp.worker_mtx);
            SOKOL_ASSERT(res == thrd_success);

            // this buffer can now be reused
            _sgp_release_command_buffer(cbuf);
        }
        // no job to do and worker was requested to stop
        else if(!_sgp.working) {
            break;
        }
        // no job to do, wait
        else {
            // notify that we are idle in case of synchronize request
            _sgp.worker_idle = true;
            res = cnd_signal(&_sgp.wake_main_cnd);
            SOKOL_ASSERT(res == thrd_success);

            // wait for jobs
            res = cnd_wait(&_sgp.wake_worker_cnd, &_sgp.worker_mtx);
            SOKOL_ASSERT(res == thrd_success);
            _sgp.worker_idle = false;
        }
    }

    // deactivate graphics context from this thread
    if(_sgp.desc.activate_cb)
        _sgp.desc.activate_cb(false);

    // notify that worker has stopped
    _sgp.worker_running = false;
    res = cnd_signal(&_sgp.wake_main_cnd);
    SOKOL_ASSERT(res == thrd_success);

    // unlock
    res = mtx_unlock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    return 0;
}

bool sgp_set_multithread(bool enable) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);

    if(_sgp.multithread == enable)
        return true;

    if(enable) {
        // initialize multi thread resources
        if(mtx_init(&_sgp.worker_mtx, mtx_plain) != thrd_success)
            return false;

        if(cnd_init(&_sgp.wake_main_cnd) != thrd_success) {
            mtx_destroy(&_sgp.worker_mtx);
            return false;
        }

        if(cnd_init(&_sgp.wake_worker_cnd) != thrd_success) {
            cnd_destroy(&_sgp.wake_main_cnd);
            mtx_destroy(&_sgp.worker_mtx);
            return false;
        }

        _sgp.multithread = true;
        _sgp.working = true;

        // deactivate the graphics context from this thread
        if(_sgp.desc.activate_cb)
            _sgp.desc.activate_cb(false);

        // start the worker thread
        if(thrd_create(&_sgp.worker, _sgp_worker_thread, NULL) != thrd_success) {
            _sgp.multithread = false;
            cnd_destroy(&_sgp.wake_worker_cnd);
            cnd_destroy(&_sgp.wake_main_cnd);
            mtx_destroy(&_sgp.worker_mtx);

            // failed, activate the graphics context on this thread again
            if(_sgp.desc.activate_cb)
                _sgp.desc.activate_cb(true);

            return false;
        }

        // lock
        int res = mtx_lock(&_sgp.worker_mtx);
        SOKOL_ASSERT(res == thrd_success); _SOKOL_UNUSED(res);

        // wait worker to start while pumping events
        while(!_sgp.worker_running)
            _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);

        // unlock
        res = mtx_unlock(&_sgp.worker_mtx);
        SOKOL_ASSERT(res == thrd_success);
    } else {
        // lock
        int res = mtx_lock(&_sgp.worker_mtx);
        SOKOL_ASSERT(res == thrd_success); _SOKOL_UNUSED(res);

        SOKOL_ASSERT(!_sgp.locked);
        _sgp.working = false;

        res = cnd_signal(&_sgp.wake_worker_cnd);
        SOKOL_ASSERT(res == thrd_success); _SOKOL_UNUSED(res);

        // wait worker to finish jobs while pumping events
        while(_sgp.worker_running)
            _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);

        // unlock
        res = mtx_unlock(&_sgp.worker_mtx);
        SOKOL_ASSERT(res == thrd_success);

        // wait the worker thread to finish all jobs
        res = thrd_join(_sgp.worker, NULL);
        SOKOL_ASSERT(res == thrd_success);

        _sgp.multithread = false;

        // free muilti thread resources
        cnd_destroy(&_sgp.wake_worker_cnd);
        cnd_destroy(&_sgp.wake_main_cnd);
        mtx_destroy(&_sgp.worker_mtx);
        memset(&_sgp.wake_worker_cnd, 0, sizeof(cnd_t));
        memset(&_sgp.wake_main_cnd, 0, sizeof(cnd_t));
        memset(&_sgp.worker_mtx, 0, sizeof(mtx_t));
        memset(&_sgp.worker, 0, sizeof(thrd_t));

        // activate graphics context to the current thread
        if(_sgp.desc.activate_cb)
            _sgp.desc.activate_cb(true);
    }

    return true;
}

bool sgp_is_multithread() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    return _sgp.multithread;
}

void sgp_synchronize() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);

    if(!_sgp.multithread)
        return;

    int res;
    _SOKOL_UNUSED(res);

    // lock
    res = mtx_lock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    // wait until worker finished and jobs and is idle
    while(_sgp.cqueue[0] != NULL || !_sgp.worker_idle) {
        _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);
    }

    // unlock
    res = mtx_unlock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);
}

void sgp_lock() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);

    if(!_sgp.multithread)
        return;

    int res;
    _SOKOL_UNUSED(res);

    // lock
    res = mtx_lock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    // wait until worker finished and jobs and is idle
    while(_sgp.cqueue[0] != NULL || !_sgp.worker_idle) {
        _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);
    }

    SOKOL_ASSERT(!_sgp.locked);
    _sgp.locked = true;

    // some backends must explicitly do a graphics context switch when changing threads
    if(_sgp.desc.activate_cb) {
        // signal the worker to deactivate the graphics context
        cnd_signal(&_sgp.wake_worker_cnd);

        // wait for it
        while(_sgp.worker_context_active)
            _sgp_cnd_wait_pumped(&_sgp.wake_main_cnd, &_sgp.worker_mtx);

        // activate graphics context on this thread
        _sgp.desc.activate_cb(true);
    }

    // unlock
    res = mtx_unlock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);
}

void sgp_unlock() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);

    if(!_sgp.multithread)
        return;

    int res;
    _SOKOL_UNUSED(res);

    res = mtx_lock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);

    SOKOL_ASSERT(_sgp.locked);
    _sgp.locked = false;

    // some backends must explicitly do a graphics context switch when changing threads
    if(_sgp.desc.activate_cb) {
        // deactivate graphics context from this thread
        _sgp.desc.activate_cb(false);

        // signal the worker to activate the graphics context on his thread
        cnd_signal(&_sgp.wake_worker_cnd);
    }

    res = mtx_unlock(&_sgp.worker_mtx);
    SOKOL_ASSERT(res == thrd_success);
}


#endif // SOKOL_GP_IMPL_INCLUDED
#endif // SOKOL_GP_IMPL
