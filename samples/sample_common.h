#define FLEXTGL_IMPL
#include "flextgl.h"
#define NANOGCTX_GLCORE33_BACKEND
#ifdef WIN32
#define NANOGCTX_D3D11_BACKEND
#endif
#define NANOGCTX_DUMMY_BACKEND
#define NANOGCTX_IMPL
#include "nanogctx.h"
#define NANOGP_IMPL
//#define NANOGP_DUMMY_BACKEND
#include "nanogp.h"
#include <SDL2/SDL.h>
#include <math.h>

int sample_app(void (*draw)(ngctx_context ngctx)) {
    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // setup context attributes before window and context creation
    ngctx_desc ctx_desc = {
#ifdef NANOGP_GLCORE33_BACKEND
        .backend = NGCTX_BACKEND_GLCORE33,
#endif
#ifdef NANOGP_D3D11_BACKEND
        .backend = NGCTX_BACKEND_D3D11,
#endif
        .sample_count = 0
    };
    ngctx_prepare_attributes(&ctx_desc);

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

    ngp_desc desc = {0};
#ifdef NANOGCTX_D3D11_BACKEND
    if(ctx_desc.backend == NGCTX_BACKEND_D3D11) {
        desc.sg.context.d3d11.device = ngctx.d3d11->device;
        desc.sg.context.d3d11.device_context = ngctx.d3d11->device_context;
        desc.sg.context.d3d11.render_target_view_cb = ngctx_d3d11_render_target_view;
        desc.sg.context.d3d11.depth_stencil_view_cb = ngctx_d3d11_depth_stencil_view;
    }
#endif

    // create NanoGP context
    if(!ngp_setup(&desc)) {
        fprintf(stderr, "Failed to create NGP context: %s\n", ngp_get_error());
        return 1;
    }

    // run loop
    while(!SDL_QuitRequested()) {
        // poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)) { }

        draw(ngctx);

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
    ngp_shutdown();
    ngctx_destroy(ngctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
