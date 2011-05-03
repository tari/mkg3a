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

u8 convertChannelDepth(u8 c, char depth) {
	float v = c / 255.0;
	v *= (1 << depth) - 1;
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
		b = convertChannelDepth(d[pxi * 3], 5);
		g = convertChannelDepth(d[pxi * 3 + 1], 6);
		r = convertChannelDepth(d[pxi * 3 + 2], 5);

		px = (r << 11) | (g << 5) | b;
		// Write as big-endian
		d[2 * pxi] = px >> 8;
		d[2 * pxi + 1] = px & 0xFF;
	}
	return realloc(d, 2 * ICON_HEIGHT * ICON_WIDTH);
}

#endif /* USE_MAGICKCORE */
