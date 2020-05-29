#include "sample_common.h"

sg_image image;
float image_ratio;

void draw(int width, int height) {
    ngp_set_clear_color(0.05f, 0.05f, 0.05f, 1.0f);
    ngp_begin(width, height);

    float ih = 512.0f;
    float iw = ih * image_ratio;
    ngp_translate((width - iw) / 2.0f, (height - ih) / 2.0f);
    ngp_draw_textured_rect(image, (ngp_rect){0, 0, iw, ih}, NULL);

/*
    for(int y=0;y<64;++y) {
        for(int x=0;x<64;++x) {
            if(x % 3 == 0)
                ngp_set_color(255, 0, 0, 255);
            else if(x % 3 == 1)
                ngp_set_color(0, 255, 0, 255);
            else
                ngp_set_color(0, 0, 255, 255);
            ngp_draw_rect(x*4, y*4, 2, 2);
            //ngp_draw_textured_rect(image, (ngp_rect){x*4, y*4, 2, 2}, NULL);
        }
    }
*/
    ngp_end();
}

bool init() {
    image = sg_load_image("resources/cat.png");
    sg_image_info imginfo = sg_query_image_info(image);
    image_ratio = imginfo.width / (float)imginfo.height;
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
