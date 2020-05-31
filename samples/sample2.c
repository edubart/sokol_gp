#include "sample_common.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#include "thirdparty/stb_image.h"

sg_image image;
float image_ratio;

void draw(int width, int height) {
    //float ih = 512.0f;
    //float iw = ih * image_ratio;
    //sgp_translate((width - iw) / 2.0f, (height - ih) / 2.0f);
    //sgp_draw_textured_rect(image, 0, 0, iw, ih);

    for(int y=0;y<32;++y) {
        for(int x=0;x<32;++x) {
            //if(x % 3 == 0)
            //    sgp_set_color(255, 0, 0, 255);
            //else if(x % 3 == 1)
            //    sgp_set_color(0, 255, 0, 255);
            //else
            //    sgp_set_color(0, 0, 255, 255);
            sgp_draw_filled_rect(x*4, y*4, 2, 2);
            sgp_draw_textured_rect(image, x*4, y*4, 2, 2);
        }
    }
}

sg_image sg_load_image(const char *filename) {
    int width, height;
    unsigned char* data = stbi_load(filename, &width, &height, NULL, 4);
    if(!data) {
        fprintf(stderr, "failed to load image '%s': stbi_load failed\n", filename);
        return (sg_image){0};
    }
    sg_image_desc image_desc = {
        .width = width,
        .height = height,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .content = {.subimage = {{{.ptr = data, .size = width * height * 4}}}},
    };
    sg_image image = sg_make_image(&image_desc);
    stbi_image_free(data);
    if(image.id == SG_INVALID_ID) {
        fprintf(stderr, "failed to load image '%s': sg_make_image failed\n", filename);
        return (sg_image){0};
    }
    return image;
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
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
