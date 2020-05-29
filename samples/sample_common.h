#define SDL_DISABLE_IMMINTRIN_H
#include <SDL2/SDL.h>
#define FLEXTGL_IMPL
#include "thirdparty/flextgl.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "thirdparty/stb_image.h"
#define NANOGCTX_IMPL
#include "nanogctx.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
//#define SOKOL_D3D11
//#define SOKOL_DUMMY_BACKEND
#include "sokol_gfx.h"
#define NANOGP_IMPL
#include "nanogp.h"
#include <stdio.h>
#include <math.h>

sg_image sg_load_image(const char *filename) {
    int width, height;
    unsigned char* data = stbi_load(filename, &width, &height, NULL, 4);
    if(!data) {
        fprintf(stderr, "failed to load image '%s': stbi_load failed\n", filename);
        return (sg_image){0};
    }
    sg_image image = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .content.subimage[0][0] = {.ptr = data, .size = width * height * 4}
    });
    stbi_image_free(data);
    if(image.id == SG_INVALID_ID) {
        fprintf(stderr, "failed to load image '%s': sg_make_image failed\n", filename);
        return (sg_image){0};
    }
    return image;
}

typedef struct sample_app_desc {
    bool (*init)();
    void (*terminate)();
    void (*draw)();
} sample_app_desc;

int sample_app(sample_app_desc app) {
    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // setup context attributes before window and context creation
    ngctx_desc ctx_desc = {
#if defined(SOKOL_GLCORE33)
        .backend = NGCTX_BACKEND_GLCORE33,
#elif defined(SOKOL_D3D11)
        .backend = NGCTX_BACKEND_D3D11,
#elif defined(SOKOL_DUMMY_BACKEND)
        .backend = NGCTX_BACKEND_DUMMY,
#endif
        .sample_count = 0
    };

    if(ctx_desc.backend == NGCTX_BACKEND_GLCORE33)
        ngctx_gl_prepare_attributes(&ctx_desc);

    // create window
    SDL_Window *window = SDL_CreateWindow("NGP Sample",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        512, 512,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    // create graphics context
    ngctx_context ngctx = {0};
    if(!ngctx_create(&ngctx, window, &ctx_desc)) {
        fprintf(stderr, "Failed to create NGCTX context: %s\n", ngctx_get_error());
        return 1;
    }
    ngctx_set_swap_interval(ngctx, 0);

    // setup sokol
    sg_desc desc = {0};
#ifdef SOKOL_D3D11
    if(ctx_desc.backend == NGCTX_BACKEND_D3D11) {
        desc.context.d3d11.device = ngctx.d3d11->device;
        desc.context.d3d11.device_context = ngctx.d3d11->device_context;
        desc.context.d3d11.render_target_view_cb = ngctx_d3d11_render_target_view;
        desc.context.d3d11.depth_stencil_view_cb = ngctx_d3d11_depth_stencil_view;
    }
#endif
    sg_setup(&desc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol context\n");
        return 1;
    }

    // setup NanoGP
    if(!ngp_setup(&(ngp_desc){.max_vertices=262144, .max_commands=32768})) {
        fprintf(stderr, "Failed to create NanoGP context: %s\n", ngp_get_error());
        return 1;
    }

    // setup app resources
    if(!app.init()) {
        fprintf(stderr, "Failed to initialize app\n");
        return 1;
    }

    // run loop
    while(!SDL_QuitRequested()) {
        ngctx_isize size = ngctx_get_drawable_size(ngctx);

        // poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)) { }

        app.draw(size.w, size.h);
        if(!ngctx_swap(ngctx)) {
            fprintf(stderr, "Failed to swap window buffers: %s\n", ngctx_get_error());
            return 1;
        }

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

    // destroy
    app.terminate();
    ngp_shutdown();
    sg_shutdown();
    ngctx_destroy(ngctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
