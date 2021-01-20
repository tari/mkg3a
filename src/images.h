#ifndef _IMAGES_H
#define _IMAGES_H
#include "config.h"
#include <stdio.h>

#pragma pack(1) /* Padding would be bad, since we oneshot from disk */
struct bmp_header {
    u8 signature[2];
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 px_offset;
};

struct dib_header {
    u32 header_size;
    int32_t width;
    int32_t height;
    u16 nplanes;
    u16 bpp;
    u32 compress_type;
    u32 bmp_byte_size;
    int32_t hres;
    int32_t vres;
    u32 ncolors;
    u32 nimpcolors;
};
#pragma pack()

u16 *loadBitmap(const char *path, int32_t *width, int32_t *height);
int readBMPHeader(struct bmp_header *bh, struct dib_header *dh, FILE *fp);
int readBMPData(struct dib_header *bh, u8 *d, FILE *fp);
u8 convertChannelDepth(u8 c, u8 cd, u8 dd);
u16 *convertBPP(int32_t w, int32_t h, u8 *d);
void writeBitmap(const char *path, u16 *data, int w, int h);

#endif // _IMAGES_H
