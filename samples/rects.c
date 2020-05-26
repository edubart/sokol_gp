#include "sample_common.h"

void draw_transforms(ngp_context* ngp) {
    int width = ngp->viewport.w, height = ngp->viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = SDL_GetTicks() / 1000.0f;
    float t = (1.0f+sinf(time))/2.0f;

    // left
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.25f - hsize, height*0.5f - hsize);
    ngp_translate(ngp, 0.0f, 2*size*t - size);
    ngp_set_color(ngp, t, 0.3f, 1.0f-t, 1.0f);
    ngp_fill_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);

    // middle
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.5f - hsize, height*0.5f - hsize);
    ngp_rotate_at(ngp, time, hsize, hsize);
    ngp_set_color(ngp, t, 1.0f - t, 0.3f, 1.0f);
    ngp_fill_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);

    // right
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.75f - hsize, height*0.5f - hsize);
    ngp_scale_at(ngp, t + 0.25f, t + 0.5f, hsize, hsize);
    ngp_set_color(ngp, 0.3f, t, 1.0f - t, 1.0f);
    ngp_fill_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);
}

void render(ng_context* ngctx, ngp_context* ngp) {
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    ngp_set_clear_color(ngp, 0.05f, 0.05f, 0.05f, 1.0f);
    ngp_begin(ngp, width, height);
    int hw = width / 2;
    int hh = height / 2;

    ngp_viewport(ngp, 0, 0, hw, hh);
    ngp_set_color(ngp, 0.1f, 0.1f, 0.1f, 1.0f);
    ngp_fill_rect(ngp, 0, 0, hw, hh);
    draw_transforms(ngp);

    ngp_viewport(ngp, hw, hh, hw, hh);
    ngp_set_color(ngp, 0.1f, 0.1f, 0.1f, 1.0f);
    ngp_fill_rect(ngp, 0, 0, hw, hh);
    ngp_scissor(ngp, 0, 0, hw, hh / 2);
    draw_transforms(ngp);

    ngp_end(ngp);
    ngctx_swap(ngctx);
}

int main() {
    return sample_app(render);
}
