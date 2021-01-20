#include "g3a.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "images.h"
#include "util.h"

/*
 * Makes a g3a file with name outFile from inFile.
 * names is an array of strings giving names to insert:
 *     short internal en es de fr pt zh en en
 * NULL names will be left blank.
 *
 * Returns 0 on success, nonzero otherwise.
 */
int g3a_mkG3A(const char *inFile, const char *outFile, struct lc_names *names,
              struct icons *icons, const char *version) {
    u32 inSize, cksum;
    struct g3a_header *header;
    const char *baseName;

    FILE *outFP = fopen(outFile, "wb");
    if (outFP == NULL) {
        printf("Unable to open output file: %s\n", strerror(errno));
        return 1;
    }

    fseek(outFP, sizeof(struct g3a_header), SEEK_SET);
    if (g3a_processRaw(inFile, outFP, &inSize, &cksum))
        return 1;

    header = g3a_mkHeader(1);
    if (header == NULL)
        return 1;
    g3a_fillSize(header, inSize);
    g3a_fillCProt(header);
    g3a_fillIcons(header, icons);
    g3a_fillVersion(header, version);

    baseName = basename(outFile);
    strncpy(header->filename, baseName, sizeof(header->filename) - 1);
    g3a_fillNames(header, names);

    cksum += checksum(header, sizeof(struct g3a_header));
    dumpb_u32(cksum, &header->cksum);
    fwrite(&header->cksum, sizeof(header->cksum), 1, outFP);

    // Write header
    fseek(outFP, 0, SEEK_SET);
    fwrite(header, sizeof(struct g3a_header), 1, outFP);

    free(header);
    fclose(outFP);
    return 0;
}

/*
 * Fills in copy protection field, depends on size
 */
void g3a_fillCProt(struct g3a_header *h) {
    u32 *cprot = (u32 *)&h->cprot[2];
    *cprot = ~h->size; // Already written big-endian once

    h->cprot[0] = h->cprot[5] - 0x41;
    h->cprot[1] = 0xFE;
    h->cprot[6] = h->cprot[5] - 0xB8;
}

/*
 * Copies icons in
 */
void g3a_fillIcons(struct g3a_header *h, struct icons *icons) {
    memcpy(h->icon_unsel, icons->unselected, sizeof(h->icon_unsel));
    memcpy(h->icon_sel, icons->selected, sizeof(h->icon_sel));
}

/*
 * Fills the name fields in h from names.  See g3a_mkG3A, short and internal
 * are required.
 */
void g3a_fillNames(struct g3a_header *h, struct lc_names *names) {
    unsigned int i;
    char *src;

    strncpy(h->name_basic, names->basic, sizeof(h->name_basic) - 1);
    if (names->internal == NULL)
        src = names->basic;
    else
        src = names->internal;
    strncpy(h->name_internal, src, sizeof(h->name_internal) - 1);
    // All-caps + leading @
    memmove(h->name_internal + 1, h->name_internal,
            sizeof(h->name_internal) - 2);
    h->name_internal[0] = '@';
    for (i = 0; i < sizeof(h->name_internal) - 1; i++) {
        h->name_internal[i] = toupper((int)h->name_internal[i]);
    }

    for (i = 0; i < sizeof(h->lc_names) / sizeof(h->name_en); i++) {
        if (names->localized[i] == NULL) {
            src = names->basic;
        } else {
            src = names->localized[i];
        }
        strncpy((char *)&h->lc_names[i], src, sizeof(h->lc_names[i]) - 1);
    }
}

/*
 * Fills in timestamp with current time
 */
void g3a_fillTimestamp(struct g3a_header *h) {
    time_t now_t = time(NULL);
    struct tm *now = localtime(&now_t);
    strftime(h->timestamp, sizeof(h->timestamp), "%Y.%m%d.%H%M", now);
}

/* Allocates a new header and fills in the normally untouched fields.
 * Specifically:
 *  - magic
 *  - size, cksum2_ofs
 *  - cprot
 *  - version (01.00.0000)
 *  - timestamp (current time) */
struct g3a_header *g3a_mkHeader(int type) {
    u8 MAGIC[] = {0xAA, 0xAC, 0xBD, 0xAF, 0x90, 0x88, 0x9A,
                  0x8D, 0xD3, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF};

    struct g3a_header *h = callocs(1, sizeof(struct g3a_header));

    memcpy(h->magic, MAGIC, sizeof(MAGIC));
    h->_pad3[0] = 0x01;
    h->_pad3[1] = 0x01; // Doesn't seem necessary, but we'll use it
    strncpy(h->version, "01.00.0000", sizeof(h->version) - 1);
    g3a_fillTimestamp(h);
    return h;
}

void g3a_fillSize(struct g3a_header *h, u32 codeSize) {
    u32 outSize;

    dumpb_u32(codeSize, &h->cksum2_ofs);
    outSize = codeSize + 0x7004;
    dumpb_u32(outSize, &h->size);
}

void g3a_fillVersion(struct g3a_header *h, const char *version) {
    strncpy(h->version, version, sizeof(h->version) - 1);
}

/*
 * Read inFile, checksum it, and write the contents into outFile.
 */
int g3a_processRaw(const char *inFile, FILE *outFile, u32 *size, u32 *cksum) {
    u32 sum = 0;
    u8 buf[FREAD_CHUNK];
    size_t rsize;
    FILE *inFP;

    inFP = fopen(inFile, "rb");
    if (inFP == NULL) {
        printf("Failed to open input file for reading!\n");
        return 1;
    }
    fseek(inFP, 0, SEEK_END);
    if (ftell(inFP) > 0x01000000) {
        printf(
            "Cowardly refusing to operating on input file larger than 16MB.\n");
        return 1;
    }
    rewind(inFP);

    *size = 0;
    do {
        // Read a chunk, update checksum, write
        rsize = fread(buf, 1, FREAD_CHUNK, inFP);
        sum += checksum(buf, rsize);
        fwrite(buf, rsize, 1, outFile);
        *size += rsize;
    } while (rsize == FREAD_CHUNK);

    fclose(inFP);
    *cksum = sum;
    return 0;
}
