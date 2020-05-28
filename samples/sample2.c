#define NANOGP_GLCORE33_BACKEND
//#define NANOGP_D3D11_BACKEND
//#define NANOGP_DUMMY_BACKEND

#include "sample_common.h"

sg_image image;

void draw(ngctx_context ngctx) {
    ngctx_isize size = ngctx_get_drawable_size(ngctx);
    ngp_set_clear_color(0.05f, 0.05f, 0.05f, 1.0f);
    ngp_begin(size.w, size.h);

    ngp_ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    ngp_draw_rect(0.0f, 0.0f, 0.5f, 0.5f);
    ngp_end();
}

bool init() {
    image = sg_load_image("resources/cat.png");
    if(image.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(image);
}

int main(int argc, char *argv[]) {
    return sample_app((sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw
    });
}
