#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "images.h"

static void usage();

int main(int argc, char **argv) {
    int32_t width = 0, height = 0;
    u16 *data;
    FILE *fp = NULL;

    if (argc < 3) {
        usage();
        return 1;
    }

    data = loadBitmap(argv[1], &width, &height);
    if (data == NULL) {
        fprintf(stderr, "Failed to read input file: %s\n", argv[1]);
        return 1;
    } else {
        printf("Loaded image, %i x %i pixels.\n", width, height);
    }

    fp = fopen(argv[2], "w");
    if (fp == NULL || fwrite(data, width * height, sizeof(u16), fp) == 0) {
        fprintf(stderr, "Failed to write output file: %s\n", argv[2]);
        return 1;
    }
    fclose(fp);
    free(data);
    return 0;
}

void usage() { fprintf(stderr, "Usage: convert565 in.bmp out.bin\n"); }
