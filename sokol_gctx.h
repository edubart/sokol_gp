/*
sokol_gctx.h - graphics context creation utility for Sokol and SDL2
https://github.com/edubart/sokol_gp
*/

#ifndef SOKOL_GCTX_INCLUDED
#define SOKOL_GCTX_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define SOKOL_GCTX_GLCORE33_BACKEND
#ifdef WIN32
#define SOKOL_GCTX_D3D11_BACKEND
#endif
#define SOKOL_GCTX_DUMMY_BACKEND

#ifndef SOKOL_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_IMPL)
#define SOKOL_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_API_DECL __declspec(dllimport)
#else
#define SOKOL_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDL_Window SDL_Window;

typedef enum sgctx_backend {
    SGCTX_BACKEND_INVALID = 0,
    SGCTX_BACKEND_GLCORE33,
    SGCTX_BACKEND_D3D11,
    SGCTX_BACKEND_DUMMY
} sgctx_backend;

typedef struct sgctx_isize {
    int w, h;
} sgctx_isize;

typedef enum sgctx_error {
    SGCTX_NO_ERROR = 0,
    SGCTX_CREATE_CONTEXT_FAILED,
    SGCTX_CREATE_RENDER_TARGET_FAILED,
    SGCTX_ACTIVATE_CONTEXT_FAILED,
    SGCTX_GRAPHICS_API_UNSUPPORTED,
    SGCTX_SWAP_FAILED,
    SGCTX_DEVICE_LOST,
    SGCTX_WMINFO_FAILED,
    SGCTX_INVALID_BACKEND
} sgctx_error;

typedef struct sgctx_desc {
    sgctx_backend backend;
    int sample_count;
    int depth_size;
} sgctx_desc;

typedef struct sgctx_error_desc {
    sgctx_error code;
    const char* message;
} sgctx_error_desc;

#ifdef SOKOL_GCTX_GLCORE33_BACKEND
typedef void* SDL_GLContext;
typedef struct sgctx_gl_context {
    uint32_t init_cookie;
    SDL_Window* window;
    SDL_GLContext context;
} sgctx_gl_context;

SOKOL_API_DECL void sgctx_gl_prepare_attributes(sgctx_desc* desc);
SOKOL_API_DECL sgctx_gl_context* sgctx_gl_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_API_DECL void sgctx_gl_destroy(sgctx_gl_context* sgctx);
SOKOL_API_DECL bool sgctx_gl_activate(sgctx_gl_context* sgctx);
SOKOL_API_DECL sgctx_isize sgctx_gl_get_drawable_size(sgctx_gl_context* sgctx);
SOKOL_API_DECL bool sgctx_gl_set_swap_interval(sgctx_gl_context* sgctx, int interval);
SOKOL_API_DECL bool sgctx_gl_swap(sgctx_gl_context* sgctx);
#endif // SOKOL_GCTX_GLCORE33_BACKEND

#ifdef SOKOL_GCTX_D3D11_BACKEND
#ifndef D3D11_NO_HELPERS
#define D3D11_NO_HELPERS
#endif
#ifndef CINTERFACE
#define CINTERFACE
#endif
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

typedef struct sgctx_d3d11_context {
    uint32_t init_cookie;
    sgctx_desc desc;
    SDL_Window* window;
    int width, height;
    int swap_interval;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    IDXGISwapChain* swap_chain;
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Texture2D* render_target;
    ID3D11RenderTargetView* render_target_view;
    ID3D11Texture2D* depth_stencil_buffer;
    ID3D11DepthStencilView* depth_stencil_view;
} sgctx_d3d11_context;

SOKOL_API_DECL sgctx_d3d11_context* sgctx_d3d11_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_API_DECL void sgctx_d3d11_destroy(sgctx_d3d11_context* sgctx);
SOKOL_API_DECL bool sgctx_d3d11_activate(sgctx_d3d11_context* sgctx);
SOKOL_API_DECL sgctx_isize sgctx_d3d11_get_drawable_size(sgctx_d3d11_context* sgctx);
SOKOL_API_DECL bool sgctx_d3d11_set_swap_interval(sgctx_d3d11_context* sgctx, int interval);
SOKOL_API_DECL bool sgctx_d3d11_swap(sgctx_d3d11_context* sgctx);
SOKOL_API_DECL const void* sgctx_d3d11_render_target_view();
SOKOL_API_DECL const void* sgctx_d3d11_depth_stencil_view();

#endif // SOKOL_GCTX_D3D11_BACKEND

#ifdef SOKOL_GCTX_DUMMY_BACKEND
typedef struct sgctx_dummy_context{
    uint32_t init_cookie;
    SDL_Window* window;
} sgctx_dummy_context;

SOKOL_API_DECL sgctx_dummy_context* sgctx_dummy_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_API_DECL void sgctx_dummy_destroy(sgctx_dummy_context* sgctx);
SOKOL_API_DECL bool sgctx_dummy_activate(sgctx_dummy_context* sgctx);
SOKOL_API_DECL sgctx_isize sgctx_dummy_get_drawable_size(sgctx_dummy_context* sgctx);
SOKOL_API_DECL bool sgctx_dummy_set_swap_interval(sgctx_dummy_context* sgctx, int interval);
SOKOL_API_DECL bool sgctx_dummy_swap(sgctx_dummy_context* sgctx);
#endif // SOKOL_GCTX_DUMMY_BACKEND

typedef struct sgctx_context {
    sgctx_backend backend;
    union {
        void *p;
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        sgctx_gl_context* gl;
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        sgctx_d3d11_context* d3d11;
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        sgctx_dummy_context* dummy;
        #endif
    };
} sgctx_context;

SOKOL_API_DECL bool sgctx_create(sgctx_context* sgctx, SDL_Window* window, sgctx_desc* desc);
SOKOL_API_DECL void sgctx_destroy(sgctx_context* sgctx);
SOKOL_API_DECL bool sgctx_activate(sgctx_context sgctx);
SOKOL_API_DECL bool sgctx_is_valid(sgctx_context sgctx);
SOKOL_API_DECL sgctx_isize sgctx_get_drawable_size(sgctx_context sgctx);
SOKOL_API_DECL bool sgctx_set_swap_interval(sgctx_context sgctx, int interval);
SOKOL_API_DECL bool sgctx_swap(sgctx_context sgctx);
SOKOL_API_DECL const char* sgctx_get_error();
SOKOL_API_DECL sgctx_error sgctx_get_error_code();

// TODO:
// SOKOL_GCTX_GLES2
// SOKOL_GCTX_GLES3
// SOKOL_GCTX_METAL
// SOKOL_GCTX_D3D11

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GCTX_INCLUDED

#ifdef SOKOL_GCTX_IMPL
#ifndef SOKOL_GCTX_IMPL_INCLUDED
#define SOKOL_GCTX_IMPL_INCLUDED

#ifndef SOKOL_ASSERT
#include <assert.h>
#define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_MALLOC
#include <stdlib.h>
#define SOKOL_MALLOC(s) malloc(s)
#define SOKOL_FREE(p) free(p)
#endif
#ifndef SOKOL_DEBUG
#ifndef NDEBUG
#define SOKOL_DEBUG
#endif
#endif
#ifndef SOKOL_LOG
#ifdef SOKOL_DEBUG
#include <stdio.h>
#define SOKOL_LOG(s) { SOKOL_ASSERT(s); puts(s); }
#else
#define SOKOL_LOG(s)
#endif
#endif
#ifndef SOKOL_UNREACHABLE
#define SOKOL_UNREACHABLE SOKOL_ASSERT(false)
#endif
#ifndef _SOKOL_UNUSED
#define _SOKOL_UNUSED(x) (void)(x)
#endif

#include <string.h>
#include <SDL2/SDL.h>

enum {
    _SGCTX_INIT_COOKIE = 0xCAFED0D
};

static sgctx_error_desc sgctx_last_error = {SGCTX_NO_ERROR, ""};

static void _sgctx_set_error(sgctx_error code, const char *message) {
    sgctx_last_error.code = code;
    sgctx_last_error.message = message;
    SOKOL_LOG(message);
}

const char* sgctx_get_error() {
    return sgctx_last_error.message;
}

sgctx_error sgctx_get_error_code() {
    return sgctx_last_error.code;
}

#ifdef SOKOL_GCTX_GLCORE33_BACKEND

void sgctx_gl_prepare_attributes(sgctx_desc* desc) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desc->depth_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desc->sample_count > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc->sample_count);
}

sgctx_gl_context* sgctx_gl_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);
    _SOKOL_UNUSED(desc);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }
    if(SDL_GL_MakeCurrent(window, context) != 0) {
        _sgctx_set_error(SGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        SDL_GL_DeleteContext(context);
        return NULL;
    }

    sgctx_gl_context* sgctx = (sgctx_gl_context*)SOKOL_MALLOC(sizeof(sgctx_gl_context));
    memset(sgctx, 0, sizeof(sgctx_gl_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->context = context;
    sgctx->window = window;
    return sgctx;
}

void sgctx_gl_destroy(sgctx_gl_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    SDL_GL_DeleteContext(sgctx->context);
    SOKOL_FREE(sgctx);
}

bool sgctx_gl_activate(sgctx_gl_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    if(SDL_GL_MakeCurrent(sgctx->window, sgctx->context) != 0) {
        _sgctx_set_error(SGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        return false;
    }
    return true;
}
sgctx_isize sgctx_gl_get_drawable_size(sgctx_gl_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    sgctx_isize size;
    SDL_GL_GetDrawableSize(sgctx->window, &size.w, &size.h);
    return size;
}

bool sgctx_gl_set_swap_interval(sgctx_gl_context* sgctx, int interval) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _SOKOL_UNUSED(sgctx);
    return SDL_GL_SetSwapInterval(interval) == 0;
}

bool sgctx_gl_swap(sgctx_gl_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    SDL_GL_SwapWindow(sgctx->window);
    return true;
}

#endif // SOKOL_GCTX_GLCORE33_BACKEND

#ifdef SOKOL_GCTX_D3D11_BACKEND

#include <SDL2/SDL_syswm.h>
#define _sgctx_safe_release(class, obj) if (obj) { class##_Release(obj); obj=0; }

static sgctx_d3d11_context* _sgctx_d3d11_active = NULL;

bool _sgctx_d3d11_create_default_render_target(sgctx_d3d11_context* sgctx) {
    HRESULT result;
    result = IDXGISwapChain_GetBuffer(sgctx->swap_chain, 0, &IID_ID3D11Texture2D, (void**)&sgctx->render_target);
    if(!SUCCEEDED(result))
        return false;

    SOKOL_ASSERT(sgctx->render_target);
    result = ID3D11Device_CreateRenderTargetView(sgctx->device, (ID3D11Resource*)sgctx->render_target, NULL, &sgctx->render_target_view);
    if(!SUCCEEDED(result))
        return false;
    SOKOL_ASSERT(sgctx->render_target_view);

    D3D11_TEXTURE2D_DESC ds_desc = {
        .Width = sgctx->width,
        .Height = sgctx->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = sgctx->swap_chain_desc.SampleDesc,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
    };
    result = ID3D11Device_CreateTexture2D(sgctx->device, &ds_desc, NULL, &sgctx->depth_stencil_buffer);
    if(!SUCCEEDED(result))
        return false;
    SOKOL_ASSERT(sgctx->depth_stencil_buffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
        .Format = ds_desc.Format,
        .ViewDimension = sgctx->desc.sample_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D
    };
    result = ID3D11Device_CreateDepthStencilView(sgctx->device, (ID3D11Resource*)sgctx->depth_stencil_buffer, &dsv_desc, &sgctx->depth_stencil_view);
    if(!SUCCEEDED(result))
        return false;
    SOKOL_ASSERT(sgctx->depth_stencil_view);
    return true;
}

void _sgctx_d3d11_destroy_default_render_target(sgctx_d3d11_context* sgctx) {
    _sgctx_safe_release(ID3D11Texture2D, sgctx->render_target);
    _sgctx_safe_release(ID3D11RenderTargetView, sgctx->render_target_view);
    _sgctx_safe_release(ID3D11Texture2D, sgctx->depth_stencil_buffer);
    _sgctx_safe_release(ID3D11DepthStencilView, sgctx->depth_stencil_view);
}

bool _sgctx_d3d11_update_default_render_target(sgctx_d3d11_context* sgctx) {
    if(!sgctx->swap_chain)
        return false;
    _sgctx_d3d11_destroy_default_render_target(sgctx);
    HRESULT result = IDXGISwapChain_ResizeBuffers(sgctx->swap_chain, 1, sgctx->width, sgctx->height, DXGI_FORMAT_UNKNOWN, 0);
    if(FAILED(result))
        return false;
    if(!_sgctx_d3d11_create_default_render_target(sgctx))
        return false;
    return true;
}

bool _sgctx_d3d11_create_device(sgctx_d3d11_context* sgctx, HWND hwnd) {
    sgctx->swap_chain_desc = (DXGI_SWAP_CHAIN_DESC) {
        .BufferDesc = {
            .Width = sgctx->width,
            .Height = sgctx->height,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .RefreshRate = {
                .Numerator = 60,
                .Denominator = 1
            }
        },
        .OutputWindow = hwnd,
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .BufferCount = 1,
        .SampleDesc = {
            .Count = sgctx->desc.sample_count > 0 ? sgctx->desc.sample_count : 1,
            .Quality = sgctx->desc.sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT
    };
    int create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
    D3D_FEATURE_LEVEL feature_level;
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        NULL,                               /* pAdapter (use default) */
        D3D_DRIVER_TYPE_HARDWARE,           /* DriverType */
        NULL,                               /* Software */
        D3D11_CREATE_DEVICE_SINGLETHREADED, /* Flags */
        NULL,                               /* pFeatureLevels */
        0,                                  /* FeatureLevels */
        D3D11_SDK_VERSION,                  /* SDKVersion */
        &sgctx->swap_chain_desc,            /* pSwapChainDesc */
        &sgctx->swap_chain,                 /* ppSwapChain */
        &sgctx->device,                     /* ppDevice */
        &feature_level,                     /* pFeatureLevel */
        &sgctx->device_context);            /* ppImmediateContext */
    if(!SUCCEEDED(result))
        return false;
    SOKOL_ASSERT(sgctx->swap_chain && sgctx->device && sgctx->device_context);
    return true;
}

void _sgctx_d3d11_destroy_device(sgctx_d3d11_context* sgctx) {
    _sgctx_safe_release(IDXGISwapChain, sgctx->swap_chain);
    _sgctx_safe_release(ID3D11DeviceContext, sgctx->device_context);
    _sgctx_safe_release(ID3D11Device, sgctx->device);
}

sgctx_d3d11_context* sgctx_d3d11_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);

    if(!SDL_GetWindowWMInfo(window, &wminfo)) {
        _sgctx_set_error(SGCTX_WMINFO_FAILED, SDL_GetError());
        return false;
    }

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    sgctx_d3d11_context* sgctx = (sgctx_d3d11_context*)SOKOL_MALLOC(sizeof(sgctx_d3d11_context));
    memset(sgctx, 0, sizeof(sgctx_d3d11_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->desc = *desc;
    sgctx->window = window;
    sgctx->width = width;
    sgctx->height = height;

    // create device and swap chain
    if(!_sgctx_d3d11_create_device(sgctx, wminfo.info.win.window)) {
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, "DirectX 11 failed to create device");
        return NULL;
    }

    // create default render target
    if(!_sgctx_d3d11_create_default_render_target(sgctx)) {
        _sgctx_set_error(SGCTX_CREATE_RENDER_TARGET_FAILED, "DirectX 11 failed to create default render target");
        _sgctx_d3d11_destroy_default_render_target(sgctx);
        _sgctx_d3d11_destroy_device(sgctx);
        SOKOL_FREE(sgctx);
        return NULL;
    }

    _sgctx_d3d11_active = sgctx;
    return sgctx;
}

void sgctx_d3d11_destroy(sgctx_d3d11_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _sgctx_d3d11_destroy_default_render_target(sgctx);
    _sgctx_d3d11_destroy_device(sgctx);
    if(_sgctx_d3d11_active == sgctx)
        _sgctx_d3d11_active = NULL;
    SOKOL_FREE(sgctx);
}

bool sgctx_d3d11_activate(sgctx_d3d11_context* sgctx) {
    _sgctx_d3d11_active = sgctx;
    return true;
}

sgctx_isize sgctx_d3d11_get_drawable_size(sgctx_d3d11_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    return (sgctx_isize){sgctx->width, sgctx->height};
}

bool sgctx_d3d11_set_swap_interval(sgctx_d3d11_context* sgctx, int interval) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    sgctx->swap_interval = interval;
    return true;
}

bool sgctx_d3d11_swap(sgctx_d3d11_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);

    unsigned int flags = sgctx->swap_interval == 0 ? DXGI_PRESENT_DO_NOT_WAIT : 0;
    HRESULT result = IDXGISwapChain_Present(sgctx->swap_chain, sgctx->swap_interval, flags);

    bool update_render_target = false;
    if(result == DXGI_ERROR_WAS_STILL_DRAWING) {
        // it's ok
    } else if(result == DXGI_ERROR_INVALID_CALL) {
        // probably went through a fullscreen <-> windowed transition
    } else if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
        // lost all resources
        _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost");
        return false;
    } else if(FAILED(result)) {
        _sgctx_set_error(SGCTX_SWAP_FAILED, "D3D11 present failed");
        return false;
    }

    // handle window resizing
    int width, height;
    SDL_GetWindowSize(sgctx->window, &width, &height);
    if(sgctx->width != width || sgctx->height != height) {
        sgctx->width = width;
        sgctx->height = height;
        if(!_sgctx_d3d11_update_default_render_target(sgctx)) {
            _sgctx_set_error(SGCTX_CREATE_RENDER_TARGET_FAILED, "D3D11 failed to recreate default render target");
            return false;
        }
    }

    return true;
}

const void* sgctx_d3d11_render_target_view() {
    if(_sgctx_d3d11_active)
        return (const void*) _sgctx_d3d11_active->render_target_view;
    return NULL;
}

const void* sgctx_d3d11_depth_stencil_view() {
    if(_sgctx_d3d11_active)
        return (const void*) _sgctx_d3d11_active->depth_stencil_view;
    return NULL;
}

#endif // SOKOL_GCTX_D3D11_BACKEND

#ifdef SOKOL_GCTX_DUMMY_BACKEND

sgctx_dummy_context* sgctx_dummy_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);
    _SOKOL_UNUSED(desc);
    sgctx_dummy_context* sgctx = (sgctx_dummy_context*)SOKOL_MALLOC(sizeof(sgctx_dummy_context));
    memset(sgctx, 0, sizeof(sgctx_dummy_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->window = window;
    return sgctx;
}

void sgctx_dummy_destroy(sgctx_dummy_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    SOKOL_FREE(sgctx);
}

bool sgctx_dummy_activate(sgctx_dummy_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _SOKOL_UNUSED(sgctx);
    return true;
}

sgctx_isize sgctx_dummy_get_drawable_size(sgctx_dummy_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    sgctx_isize size;
    SDL_GetWindowSize(sgctx->window, &size.w, &size.h);
    return size;
}

bool sgctx_dummy_set_swap_interval(sgctx_dummy_context* sgctx, int interval) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _SOKOL_UNUSED(sgctx);
    _SOKOL_UNUSED(interval);
    return true;
}

bool sgctx_dummy_swap(sgctx_dummy_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _SOKOL_UNUSED(sgctx);
    return true;
}

#endif // SOKOL_GCTX_DUMMY_BACKEND

bool sgctx_create(sgctx_context* sgctx, SDL_Window* window, sgctx_desc* desc) {
    switch(desc->backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            sgctx->gl = sgctx_gl_create(window, desc);
            sgctx->backend = SGCTX_BACKEND_GLCORE33;
            break;
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            sgctx->d3d11 = sgctx_d3d11_create(window, desc);
            sgctx->backend = SGCTX_BACKEND_D3D11;
            break;
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            sgctx->dummy = sgctx_dummy_create(window, desc);
            sgctx->backend = SGCTX_BACKEND_DUMMY;
            break;
        #endif
        default:
            sgctx->backend = SGCTX_BACKEND_INVALID;
            sgctx->p = NULL;
            _sgctx_set_error(SGCTX_INVALID_BACKEND, "invalid backend");
            break;
    }
    return sgctx->backend != SGCTX_BACKEND_INVALID;
}

void sgctx_destroy(sgctx_context* sgctx) {
    switch(sgctx->backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            sgctx_gl_destroy(sgctx->gl); break;
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            sgctx_d3d11_destroy(sgctx->d3d11); break;
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            sgctx_dummy_destroy(sgctx->dummy); break;
        #endif
        default:
            // already destroyed
            break;
    }
    *sgctx = (sgctx_context){.backend=SGCTX_BACKEND_INVALID,.p=NULL};
}

bool sgctx_activate(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
    switch(sgctx.backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            return sgctx_gl_activate(sgctx.gl);
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            return sgctx_d3d11_activate(sgctx.d3d11);
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            return sgctx_dummy_activate(sgctx.dummy);
        #endif
        default:
            SOKOL_UNREACHABLE; return false;
    }
}

bool sgctx_is_valid(sgctx_context sgctx) {
    return sgctx.backend != SGCTX_BACKEND_INVALID && sgctx.p != NULL;
}

sgctx_isize sgctx_get_drawable_size(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
    switch(sgctx.backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            return sgctx_gl_get_drawable_size(sgctx.gl);
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            return sgctx_d3d11_get_drawable_size(sgctx.d3d11);
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            return sgctx_dummy_get_drawable_size(sgctx.dummy);
        #endif
        default:
            SOKOL_UNREACHABLE; return (sgctx_isize){0,0};
    }
}

bool sgctx_set_swap_interval(sgctx_context sgctx, int interval) {
    SOKOL_ASSERT(sgctx.p);
    switch(sgctx.backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            return sgctx_gl_set_swap_interval(sgctx.gl, interval);
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            return sgctx_d3d11_set_swap_interval(sgctx.d3d11, interval);
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            return sgctx_dummy_set_swap_interval(sgctx.dummy, interval);
        #endif
        default:
            SOKOL_UNREACHABLE; return false;
    }
}

bool sgctx_swap(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
    switch(sgctx.backend) {
        #ifdef SOKOL_GCTX_GLCORE33_BACKEND
        case SGCTX_BACKEND_GLCORE33:
            return sgctx_gl_swap(sgctx.gl);
        #endif
        #ifdef SOKOL_GCTX_D3D11_BACKEND
        case SGCTX_BACKEND_D3D11:
            return sgctx_d3d11_swap(sgctx.d3d11);
        #endif
        #ifdef SOKOL_GCTX_DUMMY_BACKEND
        case SGCTX_BACKEND_DUMMY:
            return sgctx_dummy_swap(sgctx.dummy);
        #endif
        default:
            SOKOL_UNREACHABLE; return false;
    }
}

#endif // SOKOL_GCTX_IMPL_INCLUDED
#endif // SOKOL_GCTX_IMPL
