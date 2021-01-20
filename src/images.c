#include "images.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

#if USE_PNG
#include <png.h>
#endif

static char *bmperror;

#define BMPFAIL(s)                                                             \
    {                                                                          \
        bmperror = s;                                                          \
        return 1;                                                              \
    }

#if USE_PNG
u8 *readImageData_PNG(png_structp png_ptr, png_infop info_ptr,
                      int32_t *width_out, int32_t *height_out) {
    int32_t bit_depth, color_type;
    unsigned char *imageData = NULL;
    png_bytep *row_pointers = NULL;
    unsigned int y;
    size_t w, h;

    png_read_info(png_ptr, info_ptr);
    *width_out = w = png_get_image_width(png_ptr, info_ptr);
    *height_out = h = png_get_image_height(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_RGB) {
        fprintf(stderr,
                "Unsupported PNG bit depth or color type, must be RGB-8\n");
        return NULL;
    }

    // Let libpng deinterlace for us
    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // Read the file now
    imageData = mallocs((w * 3) * h);
    row_pointers = mallocs(sizeof(png_bytep) * h);
    for (y = 0; y < h; y++) {
        row_pointers[y] = imageData + (w * 3 * y);
    }
    png_read_image(png_ptr, row_pointers);

    return imageData;
}

u8 *loadBitmap_PNG(FILE *fp, int32_t *width, int32_t *height) {
    u8 *imageData = NULL;
    png_infop info_ptr = NULL;
    int px;

    // Allocate basic libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 (png_voidp)NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Failed to allocate memory for image data.");
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Failed to allocate memory for image info struct.");
        goto cleanup_png_ptr;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        // Libpng error
        goto cleanup;
    }

    png_init_io(png_ptr, fp);
    imageData = readImageData_PNG(png_ptr, info_ptr, width, height);
    if (imageData == NULL)
        goto cleanup;

    // Convert RGB pixel order to BGR expected by convertBPP later
    for (px = 0; px < *width * *height * 3; px += 3) {
        u8 t = imageData[px];
        imageData[px] = imageData[px + 2];
        imageData[px + 2] = t;
    }

cleanup:
cleanup_png_ptr:
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return imageData;
}
#endif /* USE_PNG */

u8 *loadBitmap_BMP(FILE *fp, int32_t *width, int32_t *height) {
    struct bmp_header *bh;
    struct dib_header *dh;
    int32_t w, h;
    int err = 0;
    u8 *data;

    bh = mallocs(sizeof(*bh));
    dh = mallocs(sizeof(*dh));
    if ((err = readBMPHeader(bh, dh, fp))) {
        goto out;
    }
    *width = w = dh->width;
    *height = h = dh->height;

    data = mallocs(w * 3 * h);
    err = readBMPData(dh, data, fp);

out:
    free(bh);
    free(dh);
    if (err) {
        free(data);
        printf("Error reading image: %s\n", bmperror);
        return NULL;
    }
    return data;
}

#if IS_BIG_ENDIAN
/*
 * Inverts endianness of the given DIB header.
 * Only used for big-endian systems.
 */
static void dibHeader_convert(struct dib_header *h) {
    h->header_size = u32_ntole(h->header_size);
    h->width = u32_ntole(h->width);
    h->height = u32_ntole(h->height);
    h->nplanes = u16_ntole(h->nplanes);
    h->bpp = u16_ntole(h->bpp);
    // h->compress_type = u32_flip(h->compress_type);    // != 0 fails
    // h->bmp_byte_size = u32_flip(h->bmp_byte_size);    // ignored
    // h->hres = u32_flip(h->bmp_byte_size);
    // h->vres = u32_flip(h->vres);
    // h->ncolors = u32_flip(h->ncolors);                // != 0 fails
    // h->nimpcolors = u32_flip(h->nimpcolors);            // ignored
}
#endif /* IS_BIG_ENDIAN */

int readBMPHeader(struct bmp_header *bh, struct dib_header *h, FILE *fp) {
    size_t sz;

    // BMP header
    sz = fread(bh, 1, sizeof(*bh), fp);
    if (sz != sizeof(*bh))
        BMPFAIL("Strange BMP header");
    if (bh->signature[0] != 0x42 || bh->signature[1] != 0x4D) // "BM"
        BMPFAIL("Not a BMP file");

    // DIB header
    sz = fread(h, 1, sizeof(*h), fp);
#if IS_BIG_ENDIAN
    dibHeader_convert(h);
#endif

    if (sz < sizeof(*h))
        BMPFAIL("Strange DIB header");
    if (h->nplanes != 1)
        BMPFAIL("nplanes not 1");
    if (h->bpp != 24)
        BMPFAIL("Unsupported color depth (must be 24 bpp)");
    if (h->compress_type != 0)
        BMPFAIL("Unsupported compression");
    if (h->ncolors != 0)
        BMPFAIL("Palette not supported");

    // Seek to image data
    if (fseek(fp, h->header_size - sz, SEEK_CUR) != 0)
        BMPFAIL("fseek returned abnormally");
    return 0;
}

int readBMPData(struct dib_header *dh, u8 *d, FILE *fp) {
    int row;
    size_t sz;

    // XXX hardcoded dimensions are begging to explode
    for (row = dh->height - 1; row >= 0; row--) {
        // Rows are 2208 bytes wide, so no alignment to worry about
        // (rows are supposed to be aligned to 4 bytes)
        sz = fread(d + row * dh->width * 3, 3, dh->width, fp);
        if (sz != (unsigned)dh->width)
            BMPFAIL("Unexpected EOF");
    }
    return 0;
}

/**
 * Converts channel data c from depth cd (in bits) to depth dd.
 * Behaviour is undefined if either bit depth is large (around 31 usually).
 */
u8 convertChannelDepth(u8 c, u8 cd, u8 dd) {
    float v = (float)c / ((1 << cd) - 1);
    v *= (1 << dd) - 1;
    return (u8)v;
}

/*
 * In-place conversion of data from 24bpp to 5-6-5 16bpp.
 * Also shrinks the block at d to the right size.
 */
u16 *convertBPP(int32_t w, int32_t h, u8 *d) {
    int pxi;
    u16 px;
    char r, g, b;

    for (pxi = 0; pxi < w * h; pxi++) {
        b = convertChannelDepth(d[pxi * 3], 8, 5);
        g = convertChannelDepth(d[pxi * 3 + 1], 8, 6);
        r = convertChannelDepth(d[pxi * 3 + 2], 8, 5);

        px = (r << 11) | (g << 5) | b;
        // Write as big-endian
        d[2 * pxi] = px >> 8;
        d[2 * pxi + 1] = px & 0xFF;
    }
    return realloc(d, 2 * w * h);
}

void writeBitmap(const char *path, u16 *data, int w, int h) {
    // TODO: don't write broken files on big-endian systems
    u8 *cdata;
    int x, y;
    u32 imgSize = w * h * 3;
    struct bmp_header bh = {
        {0x42, 0x4D},                                     // Signature
        imgSize + sizeof(bh) + sizeof(struct dib_header), // File size
        0,
        0,                                     // Reserved
        sizeof(bh) + sizeof(struct dib_header) // Pixel data offset
    };
    struct dib_header dh = {
        40,         // DIB header size
        w,       h, // Dimensions
        1,          // Number of planes
        24,         // BPP
        0,          // Compression
        imgSize,    // Pixel array size
        1,       1, // Pixels per meter, X/Y
        0,       0  // Color junk we don't care about
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
    cdata = mallocs(3 * w * h);
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

u16 *loadBitmap(const char *path, int32_t *width, int32_t *height) {
    u8 *imgData = NULL;
    u8 pngHeader[8];
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("Unable to open image file: %s\n", strerror(errno));
        return NULL;
    }

#if USE_PNG
    // Is this a PNG file?
    fread(pngHeader, 1, 8, fp);
    rewind(fp);
    if (0 == png_sig_cmp(pngHeader, 0, 8)) {
        // Yes, load it up
        imgData = loadBitmap_PNG(fp, width, height);
    } else {
#endif /* USE_PNG */
        if (!imgData) {
            imgData = loadBitmap_BMP(fp, width, height);
        }
#if USE_PNG
    }
#endif /* USE_PNG */

    fclose(fp);
    if (!imgData)
        return NULL;
    return convertBPP(*width, *height, imgData);
}
