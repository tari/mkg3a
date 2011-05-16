#include <stdio.h>
#include "config.h"

#ifndef _ICON_H
#define _ICON_H

#define ICON_HEIGHT 64
#define ICON_WIDTH 92
#define ICON_MONO_HEIGHT 24
#define ICON_MONO_WIDTH 64

#pragma pack(1)	/* Padding would be bad, since we oneshot from disk */
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

struct icons {
	u16 unselected[ICON_HEIGHT * ICON_WIDTH];
	u16 selected[ICON_HEIGHT * ICON_WIDTH];
	u16 mono[ICON_MONO_WIDTH * ICON_MONO_HEIGHT];
};

u16 *loadBitmap(const char *path);
int readBMPHeader(struct bmp_header *bh, struct dib_header *h, FILE *fp);
int readBMPData(u8 *d, FILE *fp);
u8 convertChannelDepth(u8 c, u8 cd, u8 dd);
u16 *convertBPP(u8 *d);
void writeBitmap(const char *path, u16 *data, int w, int h);

#endif /* _ICON_H */
