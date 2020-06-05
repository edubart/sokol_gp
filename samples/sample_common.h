#define SDL_DISABLE_IMMINTRIN_H
#include <SDL2/SDL.h>

//#define SOKOL_DUMMY_BACKEND
#ifndef SOKOL_DUMMY_BACKEND
#ifdef __WIN32__
#define SOKOL_D3D11
#else
#define SOKOL_GLCORE33
#endif
#endif

#define SOKOL_GCTX_IMPL
#define SOKOL_IMPL
#define SOKOL_GFX_EXT_IMPL
#define SOKOL_GP_IMPL
#define TINYCTHREAD_IMPL
#include "thirdparty/tinycthread.h"
//#include <threads.h>

#ifdef SOKOL_GLCORE33
#define FLEXTGL_IMPL
#include "thirdparty/flextgl.h"
#endif

#include "sokol_gfx.h"
#include "sokol_gfx_ext.h"
#include "sokol_gctx.h"
#include "sokol_gp.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

typedef struct sample_app_desc {
    bool (*init)();
    void (*terminate)();
    void (*draw)(int, int);
    void (*swap)(int, int);
    int argc;
    char **argv;
} sample_app_desc;


sgctx_context sgctx;

void sample_app_swap() {
    if(!sgctx_swap(sgctx)) {
        fprintf(stderr, "Failed to swap window buffers: %s\n", sgctx_get_error());
        SDL_Event event = {.type = SDL_QUIT };
        SDL_PushEvent(&event);
    }
}

void sample_app_activate(bool active) {
    if(!sgctx_activate(sgctx, active)) {
        fprintf(stderr, "Failed to activate graphics context: %s\n", sgctx_get_error());
        SDL_Event event = {.type = SDL_QUIT };
        SDL_PushEvent(&event);
    }
}

int sample_app(sample_app_desc app) {
    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // setup context attributes before window and context creation
    sgctx_desc ctx_desc = {
        .sample_count = 0
    };

#if defined(SOKOL_GLCORE33)
    sgctx_gl_prepare_attributes(&ctx_desc);
#endif

    // create window
    SDL_Window *window = SDL_CreateWindow("Sokol GP Sample",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        512, 512,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    // create graphics context
    sgctx = sgctx_create(window, &ctx_desc);
    if(!sgctx_is_valid(sgctx)) {
        fprintf(stderr, "Failed to create SGCTX context: %s\n", sgctx_get_error());
        return 1;
    }

    // set swap interval
    bool vsync = true;
    bool mt = true;
    for(int i=1;i<app.argc;++i) {
        if(strcmp(app.argv[i], "-no-vsync") == 0)
            vsync = false;
        if(strcmp(app.argv[i], "-no-mt") == 0)
            mt = false;
    }
    sgctx_set_swap_interval(sgctx, vsync ? 1 : 0);

#if defined(SOKOL_GLCORE33)
    // load opengl api
    if(!flextInit()) {
        fprintf(stderr, "OpenGL version 3.3 unsupported");
        return 1;
    }
#endif

    // setup sokol
    sg_desc desc = {
        .context = {.depth_format = SG_PIXELFORMAT_NONE}
    };
#if defined(SOKOL_D3D11)
    desc.context.d3d11.device = sgctx.d3d11->device;
    desc.context.d3d11.device_context = sgctx.d3d11->device_context;
    desc.context.d3d11.render_target_view_cb = sgctx_d3d11_render_target_view;
    desc.context.d3d11.depth_stencil_view_cb = sgctx_d3d11_depth_stencil_view;
#endif
    sg_setup(&desc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol context\n");
        return 1;
    }

    // setup sokol gp
    sgp_desc sample_sgp_desc = {
        .max_vertices = 262144,
        .max_commands = 32768,
        .swap_cb = sample_app_swap,
#if defined(SOKOL_GLCORE33)
        .activate_cb = sample_app_activate,
#endif
        .pump_events = SDL_PumpEvents
    };
    if(!sgp_setup(&sample_sgp_desc)) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error());
        return 1;
    }

    // setup app resources
    if(!app.init()) {
        fprintf(stderr, "Failed to initialize app\n");
        return 1;
    }

    sgp_set_multithread(mt);

    // run loop
    while(!SDL_QuitRequested()) {
        sgctx_isize size = sgctx_get_drawable_size(sgctx);

        // poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)) { }

        sg_pass_action default_pass_action = {
            .colors = {{.action = SG_ACTION_CLEAR, .val = {0.05f, 0.05f, 0.05f, 1.0f}}},
            .depth = {.action = SG_ACTION_DONTCARE},
            .stencil = {.action = SG_ACTION_DONTCARE},
        };
        sgp_begin_default_pass(&default_pass_action, size.w,  size.h);
        app.draw(size.w, size.h);
        sgp_end_pass();

#if defined(SOKOL_D3D11)
        if(sgctx_d3d11_must_update(sgctx.d3d11)) {
            sgp_synchronize();
            if(!sgctx_d3d11_update(sgctx.d3d11)) {
                fprintf(stderr, "Failed to update window context: %s\n", sgctx_get_error());
                return 1;
            }
        }
#endif

        // print FPS
        static uint32_t fps = 0;
        static uint32_t last = 0;
        uint32_t now = SDL_GetTicks();
        fps++;
        if(now >= last + 1000) {
            printf("FPS: %d\n", fps);
            last = now;
            fps = 0;
        }
    }

    sgp_set_multithread(false);

    // destroy
    app.terminate();
    sgp_shutdown();
    sg_shutdown();
    sgctx_destroy(sgctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
