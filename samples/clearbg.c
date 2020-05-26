#include "sample_common.h"

void draw(ng_context* ngctx, ngp_context* ngp) {
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    float g = fmodf(SDL_GetTicks() / 1000.0f, 1.0f);
    ngp_set_clear_color(ngp, 1, g, 0, 1);
    ngp_begin(ngp, width, height);
    ngp_end(ngp);
    ngctx_swap(ngctx);
}

int main() {
    return sample_app(draw);
}
