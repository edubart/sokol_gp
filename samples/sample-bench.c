#include "sample_app.h"

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
    for(int diagonal = 0; diagonal < 2*count - 1; ++diagonal) {
        int advance = _sg_max(diagonal - count + 1, 0);
        for(int y = diagonal - advance, x = advance; y >= 0 && x < count; --y, ++x) {
            if(x % 3 == 0)
                sgp_set_color(255, 0, 0, 255);
            else if(x % 3 == 1)
                sgp_set_color(0, 255, 0, 255);
            else
                sgp_set_color(0, 0, 255, 255);
            if((x+y) % 2 == 0)
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            else
                sgp_draw_textured_rect(image, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

void bench_sync_mixed() {
    sgp_reset_color();
    for(int y=0;y<count;++y) {
        for(int x=0;x<count;++x) {
            if((x+y) % 2 == 0) {
                sgp_set_color(255, 0, 0, 255);
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            } else {
                sgp_set_color(0, 255, 0, 255);
                sgp_draw_textured_rect(image, x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            }
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

void draw() {
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

    sgp_translate(off, 0);
    bench_sync_mixed();
}

sg_image create_image(int width, int height) {
    int size = width * height * 4;
    unsigned char* data = (unsigned char*)malloc(size);
    assert(data);
    for(int y=0;y<height;++y) {
        for(int x=0;x<width;++x) {
            data[y*width*4+x*4+0] = (x*255) / width;
            data[y*width*4+x*4+1] = (y*255) / height;
            data[y*width*4+x*4+2] = 255 - (x*y*255)/ (width*height);
            data[y*width*4+x*4+3] = 255;
        }
    }
    sg_image_desc image_desc = {
        .width = width,
        .height = height,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .content = {.subimage = {{{.ptr = data, .size = size}}}},
    };
    sg_image image = sg_make_image(&image_desc);
    free(data);
    if(image.id == SG_INVALID_ID) {
        fprintf(stderr, "failed to create imaged\n");
        return (sg_image){.id=0};
    }
    return image;
}

bool init() {
    image = create_image(128, 128);
    image2 = create_image(128, 128);
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
    return sample_app_main(&(sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
