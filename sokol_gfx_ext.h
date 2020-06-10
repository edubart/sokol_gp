/*
sokol_gfx_ext.h - extensions for sokol_gfx
https://github.com/edubart/sokol_gp
*/

#ifndef SOKOL_GFX_EXT_INCLUDED
#define SOKOL_GFX_EXT_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gfx_ext.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SOKOL_API_DECL void sg_query_image_pixels(sg_image img_id, void* pixels, int size);
SOKOL_API_DECL void sg_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size);
SOKOL_API_DECL void sg_update_texture_filter(sg_image img_id, sg_filter min_filter, sg_filter mag_filter);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GFX_EXT_INCLUDED

#ifdef SOKOL_GFX_EXT_IMPL
#ifndef SOKOL_GFX_EXT_IMPL_INCLUDED
#define SOKOL_GFX_EXT_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#include <SDL2/SDL.h>

#if defined(_SOKOL_ANY_GL)

void _sg_gl_query_image_pixels(_sg_image_t* img, void* pixels) {
#if defined(SOKOL_GLCORE33)
    SOKOL_ASSERT(img->gl.target == GL_TEXTURE_2D);
    SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
    _sg_gl_store_texture_binding(0);
    _sg_gl_bind_texture(0, img->gl.target, img->gl.tex[img->cmn.active_slot]);
    glGetTexImage(img->gl.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    _SG_GL_CHECK_ERROR();
    _sg_gl_restore_texture_binding(0);
#else
    // unsupported in GLES
    _SOKOL_UNUSED(img);
    _SOKOL_UNUSED(pixels);
    SOKOL_UNREACHABLE;
#endif
}

void _sg_gl_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
#if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES3)
    SOKOL_ASSERT(pixels);
    GLuint gl_fb;
    GLint dims[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&gl_fb);
    _SG_GL_CHECK_ERROR();
    glGetIntegerv(GL_VIEWPORT, dims);
    int cur_height = dims[3];
    y = origin_top_left ? (cur_height - (y+h)) : y;
    _SG_GL_CHECK_ERROR();
    glReadBuffer(gl_fb == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
    _SG_GL_CHECK_ERROR();
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    _SG_GL_CHECK_ERROR();
#else
    _SOKOL_UNUSED(x);
    _SOKOL_UNUSED(y);
    _SOKOL_UNUSED(w);
    _SOKOL_UNUSED(h);
    _SOKOL_UNUSED(origin_top_left);
    _SOKOL_UNUSED(pixels);
    // unsupported in GLES2
    SOKOL_UNREACHABLE;
#endif
}

void _sg_gl_update_texture_filter(_sg_image_t* img, sg_filter min_filter, sg_filter mag_filter) {
    _sg_gl_store_texture_binding(0);
    _sg_gl_bind_texture(0, img->gl.target, img->gl.tex[img->cmn.active_slot]);
    img->cmn.min_filter = min_filter;
    img->cmn.mag_filter = mag_filter;
    GLenum gl_min_filter = _sg_gl_filter(img->cmn.min_filter);
    GLenum gl_mag_filter = _sg_gl_filter(img->cmn.mag_filter);
    glTexParameteri(img->gl.target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri(img->gl.target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
    _sg_gl_restore_texture_binding(0);
}

#elif defined(SOKOL_D3D11)

static uint32_t _sg_d3d11_dxgi_format_to_sdl_pixel_format(DXGI_FORMAT dxgi_format) {
    switch(dxgi_format) {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return SDL_PIXELFORMAT_ARGB8888;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return SDL_PIXELFORMAT_RGB888;
        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}

void _sg_d3d11_query_image_pixels(_sg_image_t* img, void* pixels) {
    SOKOL_ASSERT(_sg.d3d11.ctx);
    SOKOL_ASSERT(img->d3d11.tex2d);
    HRESULT hr;
    _SOKOL_UNUSED(hr);

    // create staging texture
    ID3D11Texture2D* staging_tex = NULL;
    D3D11_TEXTURE2D_DESC staging_desc = {
        .Width = (UINT)img->cmn.width,
        .Height = (UINT)img->cmn.height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = img->d3d11.format,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Usage = D3D11_USAGE_STAGING,
        .BindFlags = 0,
        .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
        .MiscFlags = 0
    };
    hr = ID3D11Device_CreateTexture2D(_sg.d3d11.dev, &staging_desc, NULL, &staging_tex);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy pixels to staging texture
    ID3D11DeviceContext_CopySubresourceRegion(_sg.d3d11.ctx,
        (ID3D11Resource*)staging_tex,
        0, 0, 0, 0,
        (ID3D11Resource*)img->d3d11.tex2d,
        0, NULL);

    // map the staging texture's data to CPU-accessible memory
    D3D11_MAPPED_SUBRESOURCE msr = {.pData = NULL};
    hr = ID3D11DeviceContext_Map(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0, D3D11_MAP_READ, 0, &msr);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy the data into the desired buffer, converting pixels to the desired format at the same time
    int res = SDL_ConvertPixels(
        img->cmn.width, img->cmn.height,
        _sg_d3d11_dxgi_format_to_sdl_pixel_format(staging_desc.Format),
        msr.pData, msr.RowPitch,
        SDL_PIXELFORMAT_RGBA32,
        pixels, img->cmn.width * 4);
    SOKOL_ASSERT(res == 0);
    _SOKOL_UNUSED(res);

    // unmap the texture
    ID3D11DeviceContext_Unmap(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0);

    if(staging_tex) ID3D11Texture2D_Release(staging_tex);
}

void _sg_d3d11_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
    // get current render target
    ID3D11RenderTargetView* render_target_view = NULL;
    ID3D11DeviceContext_OMGetRenderTargets(_sg.d3d11.ctx, 1, &render_target_view, NULL);

    // fallback to window render target
    if(!render_target_view)
        render_target_view = (ID3D11RenderTargetView*)_sg.d3d11.rtv_cb();
    SOKOL_ASSERT(render_target_view);

    // get the back buffer texture
    ID3D11Texture2D *back_buffer = NULL;
    ID3D11View_GetResource(render_target_view, (ID3D11Resource**)&back_buffer);
    SOKOL_ASSERT(back_buffer);

    // create a staging texture to copy the screen's data to
    D3D11_TEXTURE2D_DESC staging_desc;
    ID3D11Texture2D_GetDesc(back_buffer, &staging_desc);
    staging_desc.Width = w;
    staging_desc.Height = h;
    staging_desc.BindFlags = 0;
    staging_desc.MiscFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    ID3D11Texture2D *staging_tex = NULL;
    HRESULT hr = ID3D11Device_CreateTexture2D(_sg.d3d11.dev, &staging_desc, NULL, &staging_tex);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy the desired portion of the back buffer to the staging texture
    y = (origin_top_left ? y : (_sg.d3d11.cur_height - (y + h)));
    D3D11_BOX src_box = {
        .left = (UINT)x,
        .top = (UINT)y,
        .front = 0,
        .right = (UINT)(x + w),
        .bottom = (UINT)(y + w),
        .back = 1,
    };
    ID3D11DeviceContext_CopySubresourceRegion(_sg.d3d11.ctx,
        (ID3D11Resource*)staging_tex,
        0, 0, 0, 0,
        (ID3D11Resource*)back_buffer,
        0, &src_box);

    // map the staging texture's data to CPU-accessible memory
    D3D11_MAPPED_SUBRESOURCE msr = {.pData = NULL};
    hr = ID3D11DeviceContext_Map(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0, D3D11_MAP_READ, 0, &msr);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy the data into the desired buffer, converting pixels to the desired format at the same time
    int res = SDL_ConvertPixels(
        w, h,
        _sg_d3d11_dxgi_format_to_sdl_pixel_format(staging_desc.Format),
        msr.pData, msr.RowPitch,
        SDL_PIXELFORMAT_RGBA32,
        pixels, w * 4);
    SOKOL_ASSERT(res == 0);
    _SOKOL_UNUSED(res);

    // unmap the texture
    ID3D11DeviceContext_Unmap(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0);

    if(back_buffer) ID3D11Texture2D_Release(back_buffer);
    if(staging_tex) ID3D11Texture2D_Release(staging_tex);
}

void _sg_d3d11_update_texture_filter(_sg_image_t* img, sg_filter min_filter, sg_filter mag_filter) {
    SOKOL_ASSERT(img->d3d11.tex2d || img->d3d11.tex3d);
    HRESULT hr;
    _SOKOL_UNUSED(hr);
    D3D11_SAMPLER_DESC d3d11_smp_desc;
    memset(&d3d11_smp_desc, 0, sizeof(d3d11_smp_desc));
    ID3D11SamplerState_GetDesc(img->d3d11.smp, &d3d11_smp_desc);
    ID3D11SamplerState_Release(img->d3d11.smp);
    img->cmn.min_filter = min_filter;
    img->cmn.mag_filter = mag_filter;
    d3d11_smp_desc.Filter = _sg_d3d11_filter(img->cmn.min_filter, img->cmn.mag_filter, img->cmn.max_anisotropy);
    hr = ID3D11Device_CreateSamplerState(_sg.d3d11.dev, &d3d11_smp_desc, &img->d3d11.smp);
    SOKOL_ASSERT(SUCCEEDED(hr) && img->d3d11.smp);
}

#endif

void sg_query_image_pixels(sg_image img_id, void* pixels, int size) {
    SOKOL_ASSERT(pixels);
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
    SOKOL_ASSERT(size >= (img->cmn.width * img->cmn.height * 4));
    _SOKOL_UNUSED(size);
#if defined(_SOKOL_ANY_GL)
    _sg_gl_query_image_pixels(img, pixels);
#elif defined(SOKOL_D3D11)
    _sg_d3d11_query_image_pixels(img, pixels);
#endif
}

void sg_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size) {
    SOKOL_ASSERT(pixels);
    SOKOL_ASSERT(size >= w*h);
    _SOKOL_UNUSED(size);
#if defined(_SOKOL_ANY_GL)
    _sg_gl_query_pixels(x, y, w, h, origin_top_left, pixels);
#elif defined(SOKOL_D3D11)
    _sg_d3d11_query_pixels(x, y, w, h, origin_top_left, pixels);
#endif
}

void sg_update_texture_filter(sg_image img_id, sg_filter min_filter, sg_filter mag_filter) {
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
#if defined(_SOKOL_ANY_GL)
    _sg_gl_update_texture_filter(img, min_filter, mag_filter);
#elif defined(SOKOL_D3D11)
    _sg_d3d11_update_texture_filter(img, min_filter, mag_filter);
#endif
}

#endif // SOKOL_GFX_EXT_IMPL_INCLUDED
#endif // SOKOL_GFX_EXT_IMPL
