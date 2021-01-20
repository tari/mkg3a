#include <stdio.h>

#include "g3a.h"
#include "images.h"

int main(int argc, char **argv) {
    struct g3a_header h;
    FILE *fp;

    if (argc != 2) {
        printf("Usage: icodump <g3afile>\n");
        printf("\nDumps the two icons from g3afile into uns.bmp and sel.bmp\n");
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Unable to open file for reading\n");
        return 1;
    }
    fread(&h, sizeof(h), 1, fp);
    writeBitmap("uns.bmp", h.icon_unsel, G3A_ICON_WIDTH, G3A_ICON_HEIGHT);
    writeBitmap("sel.bmp", h.icon_sel, G3A_ICON_WIDTH, G3A_ICON_HEIGHT);
    return 0;
}
