#include <stdint.h>
#include "image.h"

const Image *image_load(const void *src) {
    uint16_t width;
    uint8_t height;
    Image *out;

    width = *src++ << 8;
    width |= *src++;
    height = *src++;
    if (!(out = malloc(sizeof(Image) + 2 * width * height)))
        return NULL;

    lzf_decompress(out, src, width * height * 2);
    return out;
}
