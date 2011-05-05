#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "icon.h"

static char *bmperror;

#if USE_MAGICKCORE
#error MagickCore support not implemented.
#else

#if IS_BIG_ENDIAN
#error Bitmap handler is not endianness-independent
#endif /* IS_BIG_ENDIAN */

#define BMPFAIL(s) { bmperror = s; return 1; }

u16 *loadBitmap(const char *path) {
	struct bmp_header *bh;
	struct dib_header *dh;
	int err = 0;
	u8 *data;

	FILE *fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("Unable to open image file: %s\n", strerror(errno));
		return NULL;
	}

	bh = malloc(sizeof(*bh));
	dh = malloc(sizeof(*dh));
	assert(bh != NULL && dh != NULL);
	err = readBMPHeader(bh, dh, fp);
	free(bh);
	free(dh);
	if (err) {
		printf("Error loading image: %s\n", bmperror);
		return NULL;
	}

	data = malloc(ICON_WIDTH * 3 * ICON_HEIGHT);
	assert(data != NULL);
	if (readBMPData(data, fp)) {
		printf("Error reading image: %s\n", bmperror);
		free(data);
		return NULL;
	}

	fclose(fp);
	return convertBPP(data);
}

int readBMPHeader(struct bmp_header *bh, struct dib_header *h, FILE *fp) {
	size_t sz;

	// BMP header
	sz = fread(bh, 1, sizeof(*bh), fp);
	if (sz != sizeof(*bh))
		BMPFAIL("Strange BMP header");
	if (bh->signature != 0x4D42) // "BM" little-endian
		BMPFAIL("Not a BMP file");

	// DIB header
	sz = fread(h, 1, sizeof(*h), fp);
	if (sz < sizeof(*h))
		BMPFAIL("Strange DIB header");
	if (h->width != ICON_WIDTH || h->height != ICON_HEIGHT)
		BMPFAIL("Invalid image size");
	if (h->nplanes != 1)
		BMPFAIL("nplanes not 1");
	if (h->bpp != 24)
		BMPFAIL("Unsupported color depth (must be 24 bpp)");
	if (h->compress_type != 0)
		BMPFAIL("Unsupported compression");
	if (h->ncolors != 0)
		BMPFAIL("Palette not supported");
	// Seek to image data
	assert(fseek(fp, h->header_size - sz, SEEK_CUR) == 0);
	return 0;
}

int readBMPData(u8 *d, FILE *fp) {
	int row;
	size_t sz;

	for (row = ICON_HEIGHT - 1; row >= 0; row--) {
		// Rows are 2208 bytes wide, so no alignment to worry about
		// (rows are supposed to be aligned to 4 bytes)
		sz = fread(d + row * ICON_WIDTH * 3, 3, ICON_WIDTH, fp);
		if (sz != ICON_WIDTH)
			BMPFAIL("Unexpected EOF");
	}
	return 0;
}

u8 convertChannelDepth(u8 c, u8 cd, u8 dd) {
	float v = (float)c / ((1 << cd) - 1);
	v *= (1 << dd) - 1;
	return (u8)v;
}

/*
 * In-place conversion of data from 24bpp to 5-6-5 16bpp.
 * Also shrinks the block at d to the right size.
 */
u16 *convertBPP(u8 *d) {
	int pxi;
	u16 px;
	char r, g, b;

	for (pxi = 0; pxi < ICON_HEIGHT * ICON_WIDTH; pxi++) {
		b = convertChannelDepth(d[pxi * 3], 8, 5);
		g = convertChannelDepth(d[pxi * 3 + 1], 8, 6);
		r = convertChannelDepth(d[pxi * 3 + 2], 8, 5);

		px = (r << 11) | (g << 5) | b;
		// Write as big-endian
		d[2 * pxi] = px >> 8;
		d[2 * pxi + 1] = px & 0xFF;
	}
	return realloc(d, 2 * ICON_HEIGHT * ICON_WIDTH);
}


void writeBitmap(const char *path, u16 *data, int w, int h) {
	u8 *cdata;
	int x, y;
	u32 imgSize = w * h * 3;
	struct bmp_header bh = {
		0x4D42,												// Signature
		imgSize + sizeof(bh) + sizeof(struct dib_header),	// File size
		0, 0,												// Reserved
		sizeof(bh) + sizeof(struct dib_header)				// Pixel data offset
	};
	struct dib_header dh = {
		40,										// DIB header size
		w, h,									// Dimensions
		1,										// Number of planes
		24,										// BPP
		0,										// Compression
		imgSize,								// Pixel array size
		1, 1,									// Pixels per meter, X/Y
		0, 0									// Color junk we don't care about
	};
	FILE *fp = fopen(path, "wb");
	if (fp == NULL) {
		printf("Failed to open file for writing: %s\n", strerror(errno));
		return;
	}
	// Headers, sigh
	fwrite(&bh, sizeof(bh), 1, fp);
	fwrite(&dh, sizeof(dh), 1, fp);

	// Convert image data to 24bpp BGR and invert scan
	cdata = malloc(3 * w * h);
	assert(cdata != NULL);
	for (y = 0; y < h; y++) {
		u8 *destRow = cdata + w * 3 * (h - 1 - y);
		for (x = 0; x < w; x++) {
			u8 r, g, b;
			u16 px = data[w * y + x];
			px = (px << 8 & 0xFF00) | px >> 8;
			r = convertChannelDepth((px >> 11) & 0x1F, 5, 8);
			g = convertChannelDepth((px >> 5) & 0x3F, 6, 8);
			b = convertChannelDepth(px & 0x1F, 5, 8);
			destRow[3 * x] = b;
			destRow[3 * x + 1] = g;
			destRow[3 * x + 2] = r;
		}
	}
	fwrite(cdata, 3, w * h, fp);
	free(cdata);
	fclose(fp);
}

#endif /* USE_MAGICKCORE */
