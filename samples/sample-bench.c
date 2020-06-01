#include "sample_common.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#include "thirdparty/stb_image.h"

sg_image image;
sg_image image2;
float image_ratio;

const int count = 48;
const int rect_count = 4;

void bench_repeated_textured() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            sgp_draw_textured_rect(image, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_multiple_textured() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            sgp_draw_textured_rect(x % 2 == 0 ? image : image2, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_colored_textured() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            if(x % 3 == 0)
                sgp_set_color(255, 0, 0, 255);
            else if(x % 3 == 1)
                sgp_set_color(0, 255, 0, 255);
            else
                sgp_set_color(0, 0, 255, 255);
            sgp_draw_textured_rect(image, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_repeated_filled() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_multiple_filled() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_colored_filled() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            if(x % 3 == 0)
                sgp_set_color(255, 0, 0, 255);
            else if(x % 3 == 1)
                sgp_set_color(0, 255, 0, 255);
            else
                sgp_set_color(0, 0, 255, 255);
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_mixed() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            if(x % 3 == 0)
                sgp_set_color(255, 0, 0, 255);
            else if(x % 3 == 1)
                sgp_set_color(0, 255, 0, 255);
            else
                sgp_set_color(0, 0, 255, 255);
            if(x % 2 == 0)
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            else
                sgp_draw_textured_rect(image, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void draw_cat() {
    sgp_reset_color();
    sgp_draw_textured_rect(image, 0, 0, rect_count*count*2, rect_count*count*2);
}

void draw_rect() {
    sgp_reset_color();
    sgp_draw_filled_rect(0, 0, rect_count*count*2, rect_count*count*2);
}

void draw(int width, int height) {
    (void)width; (void)height;

    int off = count*rect_count*2;
    bench_repeated_textured();

    sgp_translate(off, 0);
    bench_multiple_textured();

    sgp_translate(off, 0);
    bench_colored_textured();

    sgp_translate(-2*off, off);
    bench_repeated_filled();

    sgp_translate(off, 0);
    bench_mixed();

    sgp_translate(off, 0);
    bench_colored_filled();

    sgp_translate(-2*off, off);
    draw_cat();

    sgp_translate(off, 0);
    draw_rect();
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
    image2 = sg_load_image("resources/cat.png");
    sg_image_info imginfo = sg_query_image_info(image);
    image_ratio = imginfo.width / (float)imginfo.height;
    if(image.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(image);
    sg_destroy_image(image2);
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
