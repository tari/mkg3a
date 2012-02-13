#include <stdint.h>
#ifndef _FXCG_IMAGE_H
#define _FXCG_IMAGE_H

typedef struct {
    uint16_t width;
    uint8_t height;
    uint16_t *data;
} Image;

#endif // _FXCG_IMAGE_H
