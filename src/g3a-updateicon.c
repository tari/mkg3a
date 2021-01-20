#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g3a.h"
#include "images.h"
#include "util.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: g3a-updateicon <g3afile> <selected> <unselected>\n");
        printf("\nChanges the icons in g3afile to the provided images\n");
        return 1;
    }

    FILE *g3a_f = fopen(argv[1], "r+b");
    if (g3a_f == NULL) {
        perror("Unable to open G3a file");
        return 1;
    }

    size_t g3a_size;
    if (fseek(g3a_f, 0, SEEK_END) != 0 || (g3a_size = ftell(g3a_f)) == -1UL) {
        perror("Unable to determine g3a file size");
        return 1;
    }
    if (g3a_size < (sizeof(struct g3a_header) + 4)) {
        printf("Input g3a is too small to be valid");
        return 1;
    }
    rewind(g3a_f);

    void *file_contents = malloc(g3a_size);
    if (file_contents == NULL) {
        printf("Unable to allocate g3a buffer\n");
        return 1;
    }
    if (fread(file_contents, g3a_size, 1, g3a_f) != 1) {
        perror("Failed to read g3a file contents");
        return 1;
    }

    // Load images
    int32_t w, h;
    u16 *sel_data, *unsel_data;
    if ((sel_data = loadBitmap(argv[2], &w, &h)) == NULL) {
        printf("Failed to load selected image");
        return 1;
    }
    if (w != G3A_ICON_WIDTH || h != G3A_ICON_HEIGHT) {
        printf("Selected image dimensions are incorrect: must be %dx%d",
               G3A_ICON_WIDTH, G3A_ICON_HEIGHT);
        return 1;
    }
    if ((unsel_data = loadBitmap(argv[3], &w, &h)) == NULL) {
        printf("Failed to load unselected image");
        return 1;
    }
    if (w != G3A_ICON_WIDTH || h != G3A_ICON_HEIGHT) {
        printf("Unselected image dimensions are incorrect: must be %dx%d",
               G3A_ICON_WIDTH, G3A_ICON_HEIGHT);
        return 1;
    }

    // Write new icons
    memcpy(file_contents + offsetof(struct g3a_header, icon_sel), sel_data,
           sizeof(*unsel_data) * (G3A_ICON_WIDTH * G3A_ICON_HEIGHT));
    memcpy(file_contents + offsetof(struct g3a_header, icon_unsel), unsel_data,
           sizeof(*unsel_data) * (G3A_ICON_WIDTH * G3A_ICON_HEIGHT));

    // Zero the checksums in the header and at end of file
    memset(file_contents + offsetof(struct g3a_header, cksum), 0, 4);
    memset(file_contents + g3a_size - 4, 0, 4);

    // Compute the new checksum and write back
    u32 cksum = checksum(file_contents, g3a_size);
    dumpb_u32(cksum, file_contents + offsetof(struct g3a_header, cksum));
    dumpb_u32(cksum, file_contents + g3a_size - 4);

    // Write updated file
    rewind(g3a_f);
    if (fwrite(file_contents, g3a_size, 1, g3a_f) != 1) {
        perror("Failed to write modified g3a");
        return 1;
    }

    return 0;
}
