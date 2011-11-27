#include <stdlib.h>
#include <stdio.h>
#include "icon.h"

static void usage();

int main(int argc, char **argv) {
    if (argc < 3) {
        usage();
        return 1;
    }

    size_t data_sz = sizeof(u16) * ICON_WIDTH * ICON_HEIGHT;
    int32_t width = 0, height = 0;
    u16 *data = malloc(data_sz);
    data = loadBitmap(argv[1], &width, &height);
    if (data == NULL) {
        fprintf(stderr, "Failed to read input file: %s\n", argv[1]);
        return 1;
    } else {
        printf("Loaded image, %i x %i pixels.\n", width, height);
    }

    FILE *fp = fopen(argv[2], "w");
    if (fp == NULL || fwrite(data, data_sz, 1, fp) == 0) {
        fprintf(stderr, "Failed to write output file: %s\n", argv[2]);
        return 1;
    }
    fclose(fp);
    free(data);
    return 0;
}

void usage() {
    fprintf(stderr, "Usage: convert565 in.bmp out.bin\n");
}
