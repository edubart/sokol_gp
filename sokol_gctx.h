/*
sokol_gctx.h - graphics context creation utility for Sokol and SDL2
https://github.com/edubart/sokol_gp
*/

#if defined(SOKOL_IMPL) && !defined(SOKOL_GCTX_IMPL)
#define SOKOL_GCTX_IMPL
#endif

#ifndef SOKOL_GCTX_INCLUDED
#define SOKOL_GCTX_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gctx.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDL_Window SDL_Window;

typedef struct sgctx_isize {
    int w, h;
} sgctx_isize;

typedef enum sgctx_error {
    SGCTX_NO_ERROR = 0,
    SGCTX_ALLOC_FAILED,
    SGCTX_CREATE_CONTEXT_FAILED,
    SGCTX_CREATE_RENDER_TARGET_FAILED,
    SGCTX_ACTIVATE_CONTEXT_FAILED,
    SGCTX_SWAP_FAILED,
    SGCTX_DEVICE_LOST,
    SGCTX_WMINFO_FAILED,
    SGCTX_INVALID_BACKEND
} sgctx_error;

typedef struct sgctx_desc {
    int sample_count;
    int depth_size;
} sgctx_desc;

typedef struct sgctx_error_desc {
    sgctx_error code;
    const char* message;
} sgctx_error_desc;

#if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)

typedef void* SDL_GLContext;
typedef struct sgctx_gl_context {
    uint32_t init_cookie;
    SDL_Window* window;
    SDL_GLContext context;
} sgctx_gl_context;


SOKOL_GFX_API_DECL void sgctx_gl_prepare_attributes(sgctx_desc* desc, sg_backend backend);
SOKOL_GFX_API_DECL sgctx_gl_context* sgctx_gl_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_GFX_API_DECL void sgctx_gl_destroy(sgctx_gl_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_gl_activate(sgctx_gl_context* sgctx);
SOKOL_GFX_API_DECL sgctx_isize sgctx_gl_get_drawable_size(sgctx_gl_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_gl_set_swap_interval(sgctx_gl_context* sgctx, int interval);
SOKOL_GFX_API_DECL bool sgctx_gl_swap(sgctx_gl_context* sgctx);

#elif defined(SOKOL_D3D11)

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

SOKOL_GFX_API_DECL sgctx_d3d11_context* sgctx_d3d11_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_GFX_API_DECL void sgctx_d3d11_destroy(sgctx_d3d11_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_d3d11_activate(sgctx_d3d11_context* sgctx);
SOKOL_GFX_API_DECL sgctx_isize sgctx_d3d11_get_drawable_size(sgctx_d3d11_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_d3d11_set_swap_interval(sgctx_d3d11_context* sgctx, int interval);
SOKOL_GFX_API_DECL bool sgctx_d3d11_swap(sgctx_d3d11_context* sgctx);
SOKOL_GFX_API_DECL const void* sgctx_d3d11_render_target_view();
SOKOL_GFX_API_DECL const void* sgctx_d3d11_depth_stencil_view();

#elif defined(SOKOL_METAL)

#include <SDL2/SDL_metal.h>

typedef struct sgctx_metal_context {
    uint32_t init_cookie;
    sgctx_desc desc;
    SDL_Window* window;
    SDL_MetalView view;
    void* device;

} sgctx_metal_context;

SOKOL_GFX_API_DECL sgctx_metal_context* sgctx_metal_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_GFX_API_DECL void sgctx_metal_destroy(sgctx_metal_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_metal_activate(sgctx_metal_context* sgctx);
SOKOL_GFX_API_DECL sgctx_isize sgctx_metal_get_drawable_size(sgctx_metal_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_metal_set_swap_interval(sgctx_metal_context* sgctx, int interval);
SOKOL_GFX_API_DECL bool sgctx_metal_swap(sgctx_metal_context* sgctx);
SOKOL_GFX_API_DECL const void* sgctx_metal_default_renderpass_descriptor();
SOKOL_GFX_API_DECL const void* sgctx_metal_current_drawable();

#elif defined(SOKOL_DUMMY_BACKEND)

typedef struct sgctx_dummy_context{
    uint32_t init_cookie;
    SDL_Window* window;
} sgctx_dummy_context;

SOKOL_GFX_API_DECL sgctx_dummy_context* sgctx_dummy_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_GFX_API_DECL void sgctx_dummy_destroy(sgctx_dummy_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_dummy_activate(sgctx_dummy_context* sgctx);
SOKOL_GFX_API_DECL sgctx_isize sgctx_dummy_get_drawable_size(sgctx_dummy_context* sgctx);
SOKOL_GFX_API_DECL bool sgctx_dummy_set_swap_interval(sgctx_dummy_context* sgctx, int interval);
SOKOL_GFX_API_DECL bool sgctx_dummy_swap(sgctx_dummy_context* sgctx);

#else
#error "Please define one of SOKOL_GLCORE33, SOKOL_GLES2, SOKOL_GLES3, SOKOL_D3D11, SOKOL_METAL, SOKOL_WGPU or SOKOL_DUMMY_BACKEND!"
#endif

typedef struct sgctx_context {
    union {
        void *p;
#if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
        sgctx_gl_context* gl;
#elif defined(SOKOL_D3D11)
        sgctx_d3d11_context* d3d11;
#elif defined(SOKOL_METAL)
        sgctx_metal_context* metal;
#elif defined(SOKOL_DUMMY_BACKEND)
        sgctx_dummy_context* dummy;
#endif
    };
} sgctx_context;

SOKOL_GFX_API_DECL sgctx_context sgctx_create(SDL_Window* window, sgctx_desc* desc);
SOKOL_GFX_API_DECL void sgctx_destroy(sgctx_context sgctx);
SOKOL_GFX_API_DECL bool sgctx_activate(sgctx_context sgctx);
SOKOL_GFX_API_DECL bool sgctx_is_valid(sgctx_context sgctx);
SOKOL_GFX_API_DECL sgctx_isize sgctx_get_drawable_size(sgctx_context sgctx);
SOKOL_GFX_API_DECL bool sgctx_set_swap_interval(sgctx_context sgctx, int interval);
SOKOL_GFX_API_DECL bool sgctx_swap(sgctx_context sgctx);
SOKOL_GFX_API_DECL const char* sgctx_get_error();
SOKOL_GFX_API_DECL sgctx_error sgctx_get_error_code();
SOKOL_GFX_API_DECL sg_backend sgctx_query_backend();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GCTX_INCLUDED

#ifdef SOKOL_GCTX_IMPL
#ifndef SOKOL_GCTX_IMPL_INCLUDED
#define SOKOL_GCTX_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gctx.h implementation"
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

#if defined(_SOKOL_ANY_GL)

void sgctx_gl_prepare_attributes(sgctx_desc* desc, sg_backend backend) {
    if(backend == SG_BACKEND_GLES2) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    } else if(backend == SG_BACKEND_GLES3) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    } else {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desc->depth_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desc->sample_count > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc->sample_count);
}

sgctx_gl_context* sgctx_gl_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);
    _SOKOL_UNUSED(desc);

    sgctx_gl_context* sgctx = (sgctx_gl_context*)SOKOL_MALLOC(sizeof(sgctx_gl_context));
    if(!sgctx) {
        _sgctx_set_error(SGCTX_ALLOC_FAILED, "SGCTX alloc failed");
        return NULL;
    }
    memset(sgctx, 0, sizeof(sgctx_gl_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->window = window;

    sgctx->context = SDL_GL_CreateContext(window);
    if(!sgctx->context) {
        sgctx_gl_destroy(sgctx);
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }
    if(SDL_GL_MakeCurrent(window, sgctx->context) != 0) {
        sgctx_gl_destroy(sgctx);
        _sgctx_set_error(SGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }

    return sgctx;
}

void sgctx_gl_destroy(sgctx_gl_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    if(sgctx->context)
        SDL_GL_DeleteContext(sgctx->context);
    *sgctx = (sgctx_gl_context){.init_cookie=0};
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

#elif defined(SOKOL_D3D11)

static inline HRESULT _sgctx_dxgi_GetBuffer(IDXGISwapChain* self, UINT Buffer, REFIID riid, void** ppSurface) {
    #if defined(__cplusplus)
        return self->GetBuffer(Buffer, riid, ppSurface);
    #else
        return self->lpVtbl->GetBuffer(self, Buffer, riid, ppSurface);
    #endif
}

static inline HRESULT _sgctx_dxgi_ResizeBuffers(IDXGISwapChain* self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    #if defined(__cplusplus)
        return self->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    #else
        return self->lpVtbl->ResizeBuffers(self, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    #endif
}

static inline HRESULT _sgctx_dxgi_Present(IDXGISwapChain* self, UINT SyncInterval, UINT Flags) {
    #if defined(__cplusplus)
        return self->Present(SyncInterval, Flags);
    #else
        return self->lpVtbl->Present(self, SyncInterval, Flags);
    #endif
}


static inline HRESULT _sgctx_d3d11_GetDeviceRemovedReason(ID3D11Device* self) {
    #if defined(__cplusplus)
        return self->GetDeviceRemovedReason();
    #else
        return self->lpVtbl->GetDeviceRemovedReason(self);
    #endif
}

#include <SDL2/SDL_syswm.h>

static sgctx_d3d11_context* _sgctx_d3d11_active = NULL;

bool _sgctx_d3d11_create_default_render_target(sgctx_d3d11_context* sgctx) {
    HRESULT result;
    #ifdef __cplusplus
    result = _sgctx_dxgi_GetBuffer(sgctx->swap_chain, 0, IID_ID3D11Texture2D, (void**)&sgctx->render_target);
    #else
    result = _sgctx_dxgi_GetBuffer(sgctx->swap_chain, 0, &IID_ID3D11Texture2D, (void**)&sgctx->render_target);
    #endif

    if(!SUCCEEDED(result))
        return false;

    SOKOL_ASSERT(sgctx->render_target);
    result = _sg_d3d11_CreateRenderTargetView(sgctx->device, (ID3D11Resource*)sgctx->render_target, NULL, &sgctx->render_target_view);
    if(!SUCCEEDED(result) || !sgctx->render_target_view)
        return false;

    D3D11_TEXTURE2D_DESC ds_desc = {
        .Width = (UINT)sgctx->width,
        .Height = (UINT)sgctx->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = sgctx->swap_chain_desc.SampleDesc,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
    };
    result = _sg_d3d11_CreateTexture2D(sgctx->device, &ds_desc, NULL, &sgctx->depth_stencil_buffer);
    if(!SUCCEEDED(result) || !sgctx->depth_stencil_buffer)
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
        .Format = ds_desc.Format,
        .ViewDimension = sgctx->desc.sample_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D
    };
    result = _sg_d3d11_CreateDepthStencilView(sgctx->device, (ID3D11Resource*)sgctx->depth_stencil_buffer, &dsv_desc, &sgctx->depth_stencil_view);
    if(!SUCCEEDED(result) || !sgctx->depth_stencil_view)
        return false;
    return true;
}

void _sgctx_d3d11_destroy_default_render_target(sgctx_d3d11_context* sgctx) {
    _sg_d3d11_Release(sgctx->render_target); sgctx->render_target = NULL;
    _sg_d3d11_Release(sgctx->render_target_view); sgctx->render_target_view = NULL;
    _sg_d3d11_Release(sgctx->depth_stencil_buffer); sgctx->depth_stencil_buffer = NULL;
    _sg_d3d11_Release(sgctx->depth_stencil_view); sgctx->depth_stencil_view = NULL;
}

bool _sgctx_d3d11_update_default_render_target(sgctx_d3d11_context* sgctx) {
    if(!sgctx->swap_chain)
        return false;
    _sgctx_d3d11_destroy_default_render_target(sgctx);
    HRESULT result = _sgctx_dxgi_ResizeBuffers(sgctx->swap_chain, 1, sgctx->width, sgctx->height, DXGI_FORMAT_UNKNOWN, 0);
    if(FAILED(result))
        return false;
    if(!_sgctx_d3d11_create_default_render_target(sgctx))
        return false;
    return true;
}

bool _sgctx_d3d11_create_device(sgctx_d3d11_context* sgctx, HWND hwnd) {
    sgctx->swap_chain_desc = (DXGI_SWAP_CHAIN_DESC) {
        .BufferDesc = {
            .Width = (UINT)sgctx->width,
            .Height = (UINT)sgctx->height,
            .RefreshRate = {
                .Numerator = 60,
                .Denominator = 1
            },
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM
        },
        .SampleDesc = {
            .Count = (UINT)(sgctx->desc.sample_count > 0 ? sgctx->desc.sample_count : 1),
            .Quality = (UINT)(sgctx->desc.sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0)
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 1,
        .OutputWindow = hwnd,
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
    };
    D3D_FEATURE_LEVEL feature_level;
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        NULL,                               /* pAdapter (use default) */
        D3D_DRIVER_TYPE_HARDWARE,           /* DriverType */
        NULL,                               /* Software */
        D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT, /* Flags */
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
    if(!sgctx->swap_chain || !sgctx->device || !sgctx->device_context)
        return false;
    return true;
}

void _sgctx_d3d11_destroy_device(sgctx_d3d11_context* sgctx) {
    _sg_d3d11_Release(sgctx->swap_chain); sgctx->swap_chain = NULL;
    _sg_d3d11_Release(sgctx->device_context); sgctx->device_context = NULL;
    _sg_d3d11_Release(sgctx->device); sgctx->device = NULL;
}

sgctx_d3d11_context* sgctx_d3d11_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);

    if(!SDL_GetWindowWMInfo(window, &wminfo)) {
        _sgctx_set_error(SGCTX_WMINFO_FAILED, SDL_GetError());
        return NULL;
    }

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    sgctx_d3d11_context* sgctx = (sgctx_d3d11_context*)SOKOL_MALLOC(sizeof(sgctx_d3d11_context));
    if(!sgctx) {
        _sgctx_set_error(SGCTX_ALLOC_FAILED, "SGCTX alloc failed");
        return NULL;
    }
    memset(sgctx, 0, sizeof(sgctx_d3d11_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->desc = *desc;
    sgctx->window = window;
    sgctx->width = width;
    sgctx->height = height;

    // create device and swap chain
    if(!_sgctx_d3d11_create_device(sgctx, wminfo.info.win.window)) {
        sgctx_d3d11_destroy(sgctx);
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, "D3D11 failed to create device");
        return NULL;
    }

    // create default render target
    if(!_sgctx_d3d11_create_default_render_target(sgctx)) {
        sgctx_d3d11_destroy(sgctx);
        _sgctx_set_error(SGCTX_CREATE_RENDER_TARGET_FAILED, "D3D11 failed to create default render target");
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
    *sgctx = (sgctx_d3d11_context){.init_cookie=0};
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
    HRESULT result = _sgctx_dxgi_Present(sgctx->swap_chain, sgctx->swap_interval, flags);

    if(result == DXGI_ERROR_WAS_STILL_DRAWING) {
        // it's ok
    } else if(result == DXGI_ERROR_INVALID_CALL) {
        // probably went through a fullscreen <-> windowed transition
    } else if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
        switch(_sgctx_d3d11_GetDeviceRemovedReason(sgctx->device)) {
            case DXGI_ERROR_DEVICE_HUNG:
                _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost - hung");
                return false; // lost all resources
            case DXGI_ERROR_DEVICE_REMOVED:
                _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost - removed");
                return false; // lost all resources
            case DXGI_ERROR_DEVICE_RESET:
                _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost - reset");
                return false; // lost all resources
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
                _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost - internal error");
                return false; // lost all resources
            case DXGI_ERROR_INVALID_CALL:
                _sgctx_set_error(SGCTX_DEVICE_LOST, "D3D11 device lost - invalid call");
                return false; // lost all resources
            default: // any other state should be recoverable (e.g. S_OK)
                break;
        }
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

#elif defined(SOKOL_METAL)

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

static id<MTLDevice> _sgctx_metal_device;
static CAMetalLayer* _sgctx_metal_layer;
static MTLRenderPassDescriptor* _sgctx_metal_default_renderpass_descriptor;
static id<CAMetalDrawable> _sgctx_metal_current_drawable;

sgctx_metal_context* sgctx_metal_create(SDL_Window* window, sgctx_desc* desc) {  @autoreleasepool {
    SOKOL_ASSERT(window);
    _sgctx_metal_device = MTLCreateSystemDefaultDevice();
    if(!_sgctx_metal_device) {
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, "Failed to create Metal device");
        return NULL;
    }

    SDL_MetalView view = SDL_Metal_CreateView(*window);
    if(!view) {
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }

    _sgctx_metal_layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(view);
    if(!_sgctx_metal_layer) {
        SDL_Metal_DestroyView(view);
        _sgctx_set_error(SGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }

    // Metal layer properties
    _sgctx_metal_layer.device = _sgctx_metal_device;
    _sgctx_metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    _sgctx_metal_layer.framebufferOnly = YES;
    _sgctx_metal_layer.allowsNextDrawableTimeout = NO;
    _sgctx_metal_layer.magnificationFilter = kCAFilterNearest;

    // render pass descriptor is constant
    _sgctx_metal_default_renderpass_descriptor = [[MTLRenderPassDescriptor alloc] init];

    sgctx_metal_context* sgctx = (sgctx_metal_context*)SOKOL_MALLOC(sizeof(sgctx_metal_context));
    if(!sgctx) {
        SDL_Metal_DestroyView(view);
        _sgctx_set_error(SGCTX_ALLOC_FAILED, "SGCTX alloc failed");
        return NULL;
    }

    memset(sgctx, 0, sizeof(sgctx_metal_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->desc = *desc;
    sgctx->window = *window;
    sgctx->view = view;
    sgctx->device = (__bridge void*)_sgctx_metal_device;
    return sgctx;
}}

void sgctx_metal_destroy(sgctx_metal_context* sgctx) { @autoreleasepool {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    _sgctx_metal_current_drawable = nil;
    _sgctx_metal_default_renderpass_descriptor = nil;
    _sgctx_metal_layer = nil;
    _sgctx_metal_device = nil;
    SDL_Metal_DestroyView(sgctx->view);
    *sgctx = (sgctx_metal_context){.init_cookie=0};
    SOKOL_FREE(sgctx);
}}

bool sgctx_metal_activate(sgctx_metal_context* sgctx) { @autoreleasepool {
    return true;
}}

sgctx_isize sgctx_metal_get_drawable_size(sgctx_metal_context* sgctx) { @autoreleasepool {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    int width, height;
    SDL_Metal_GetDrawableSize(sgctx->window, &width, &height);
    return (sgctx_isize){width, height};
}}

bool sgctx_metal_set_swap_interval(sgctx_metal_context* sgctx, int interval) { @autoreleasepool {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    if(interval)
        _sgctx_metal_layer.displaySyncEnabled = YES;
    else
        _sgctx_metal_layer.displaySyncEnabled = NO;
    return true;
}}

bool sgctx_metal_swap(sgctx_metal_context* sgctx) { @autoreleasepool {
    _sgctx_metal_default_renderpass_descriptor.colorAttachments[0].texture = nil;
    _sgctx_metal_current_drawable = nil;
    return true;
}}

const void* sgctx_metal_default_renderpass_descriptor() {
    _sgctx_metal_current_drawable = [_sgctx_metal_layer nextDrawable];
    _sgctx_metal_default_renderpass_descriptor.colorAttachments[0].texture = _sgctx_metal_current_drawable.texture;
    return (__bridge const void*) _sgctx_metal_default_renderpass_descriptor;
}

const void* sgctx_metal_current_drawable() {
    return (__bridge const void*) _sgctx_metal_current_drawable;
}

#elif defined(SOKOL_DUMMY_BACKEND)

sgctx_dummy_context* sgctx_dummy_create(SDL_Window* window, sgctx_desc* desc) {
    SOKOL_ASSERT(window);
    _SOKOL_UNUSED(desc);
    sgctx_dummy_context* sgctx = (sgctx_dummy_context*)SOKOL_MALLOC(sizeof(sgctx_dummy_context));
    if(!sgctx) {
        _sgctx_set_error(SGCTX_ALLOC_FAILED, "SGCTX alloc failed");
        return NULL;
    }
    memset(sgctx, 0, sizeof(sgctx_dummy_context));
    sgctx->init_cookie = _SGCTX_INIT_COOKIE;
    sgctx->window = window;
    return sgctx;
}

void sgctx_dummy_destroy(sgctx_dummy_context* sgctx) {
    SOKOL_ASSERT(sgctx->init_cookie == _SGCTX_INIT_COOKIE);
    *sgctx = (sgctx_dummy_context){.init_cookie=0};
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

#endif // SOKOL_DUMMY_BACKEND

sgctx_context sgctx_create(SDL_Window* window, sgctx_desc* desc) {
    sgctx_context sgctx;
#if defined(_SOKOL_ANY_GL)
    sgctx.gl = sgctx_gl_create(window, desc);
#elif defined(SOKOL_D3D11)
    sgctx.d3d11 = sgctx_d3d11_create(window, desc);
#elif defined(SOKOL_METAL)
    sgctx.metal = sgctx_metal_create(window, desc);
#elif defined(SOKOL_DUMMY_BACKEND)
    sgctx.dummy = sgctx_dummy_create(window, desc);
#endif
    return sgctx;
}

void sgctx_destroy(sgctx_context sgctx) {
#if defined(_SOKOL_ANY_GL)
    sgctx_gl_destroy(sgctx.gl);
#elif defined(SOKOL_D3D11)
    sgctx_d3d11_destroy(sgctx.d3d11);
#elif defined(SOKOL_METAL)
    sgctx_metal_destroy(sgctx.metal);
#elif defined(SOKOL_DUMMY_BACKEND)
    sgctx_dummy_destroy(sgctx.dummy);
#endif
}

bool sgctx_activate(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
#if defined(_SOKOL_ANY_GL)
    return sgctx_gl_activate(sgctx.gl);
#elif defined(SOKOL_D3D11)
    return sgctx_d3d11_activate(sgctx.d3d11);
#elif defined(SOKOL_METAL)
    return sgctx_metal_activate(sgctx.metal);
#elif defined(SOKOL_DUMMY_BACKEND)
    return sgctx_dummy_activate(sgctx.dummy);
#endif
}

bool sgctx_is_valid(sgctx_context sgctx) {
    if(sgctx.p == NULL)
        return false;
#if defined(_SOKOL_ANY_GL)
    return sgctx.gl->init_cookie == _SGCTX_INIT_COOKIE;
#elif defined(SOKOL_D3D11)
    return sgctx.d3d11->init_cookie == _SGCTX_INIT_COOKIE;
#elif defined(SOKOL_METAL)
    return sgctx.metal->init_cookie == _SGCTX_INIT_COOKIE;
#elif defined(SOKOL_DUMMY_BACKEND)
    return sgctx.dummy->init_cookie == _SGCTX_INIT_COOKIE;
#endif
}

sgctx_isize sgctx_get_drawable_size(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
#if defined(_SOKOL_ANY_GL)
    return sgctx_gl_get_drawable_size(sgctx.gl);
#elif defined(SOKOL_D3D11)
    return sgctx_d3d11_get_drawable_size(sgctx.d3d11);
#elif defined(SOKOL_METAL)
    return sgctx_metal_get_drawable_size(sgctx.metal);
#elif defined(SOKOL_DUMMY_BACKEND)
    return sgctx_dummy_get_drawable_size(sgctx.dummy);
#endif
}

bool sgctx_set_swap_interval(sgctx_context sgctx, int interval) {
    SOKOL_ASSERT(sgctx.p);
#if defined(_SOKOL_ANY_GL)
    return sgctx_gl_set_swap_interval(sgctx.gl, interval);
#elif defined(SOKOL_D3D11)
    return sgctx_d3d11_set_swap_interval(sgctx.d3d11, interval);
#elif defined(SOKOL_METAL)
    return sgctx_metal_set_swap_interval(sgctx.metal, interval);
#elif defined(SOKOL_DUMMY_BACKEND)
    return sgctx_dummy_set_swap_interval(sgctx.dummy, interval);
#endif
}

bool sgctx_swap(sgctx_context sgctx) {
    SOKOL_ASSERT(sgctx.p);
#if defined(_SOKOL_ANY_GL)
    return sgctx_gl_swap(sgctx.gl);
#elif defined(SOKOL_D3D11)
    return sgctx_d3d11_swap(sgctx.d3d11);
#elif defined(SOKOL_METAL)
    return sgctx_metal_swap(sgctx.metal);
#elif defined(SOKOL_DUMMY_BACKEND)
    return sgctx_dummy_swap(sgctx.dummy);
#endif
}

sg_backend sgctx_query_backend() {
#if defined(_SOKOL_ANY_GL)
    return SG_BACKEND_GLCORE33;
#elif defined(SOKOL_D3D11)
    return SG_BACKEND_D3D11;
#elif defined(SOKOL_METAL)
    return SG_BACKEND_METAL_MACOS;
#elif defined(SOKOL_DUMMY_BACKEND)
    return SG_BACKEND_DUMMY;
#endif
}

#endif // SOKOL_GCTX_IMPL_INCLUDED
#endif // SOKOL_GCTX_IMPL
