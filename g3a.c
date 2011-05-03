#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "g3a.h"
#include "icon.h"
#include "util.h"

/*
 * Makes a g3a file with name outFile from inFile.
 * names is an array of strings giving names to insert:
 *		short internal en es de fr pt zh en en
 * NULL names will be left blank.
 */
int g3a_mkG3A(const char *inFile, const char *outFile,
			  struct lc_names *names, struct icons *icons) {
    u32 inSize, cksum;
    struct g3a_header *header;
    const char *baseName;

    FILE *outFP = fopen(outFile, "wb");
    if (!outFP) {
        printf("Unable to open output file: %s\n", strerror(errno));
        return 1;
    }

    fseek(outFP, sizeof(struct g3a_header), SEEK_SET);
    cksum = g3a_processRaw(inFile, outFP, &inSize);

    header = g3a_mkHeader(1);
    g3a_fillSize(header, inSize);
    g3a_fillCProt(header, 2);
    g3a_fillIcons(header, icons);

    baseName = basename(outFile);
    strncpy(header->filename, baseName, sizeof(header->filename) - 1);
	g3a_fillNames(header, names);

    cksum += checksum(header, sizeof(struct g3a_header));
    dump_u32(cksum, &header->cksum);
	fwrite(&header->cksum, sizeof(header->cksum), 1, outFP);

	// Write header
    fseek(outFP, 0, SEEK_SET);
    fwrite(header, sizeof(struct g3a_header), 1, outFP);

    fclose(outFP);
	return 0;
}

/*
 * Fills in copy protection field, depends on size
 */
void g3a_fillCProt(struct g3a_header *h, int type) {
    u32 *cprot = (u32 *)&h->cprot[2];

    assert(type == 1 || type == 2);
    if (type == 1) {
        h->cprot[0] = 0x16;
        h->cprot[6] = 0x9F;
    } else {
        h->cprot[0] = 0xC2;
        h->cprot[6] = 0x4B;
    }
    h->cprot[1] = 0xFE;

    *cprot = ~h->size;  // Already written big-endian once
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
	int i;
	char *src;

	strncpy(h->name_basic, names->basic, sizeof(h->name_basic) - 1);
	if (names->internal == NULL)
		src = names->basic;
	else
		src = names->internal;
	strncpy(h->name_internal, src, sizeof(h->name_internal) - 1);
	// All-caps + leading @
	memmove(h->name_internal + 1, h->name_internal,
			sizeof(h->name_internal - 2));
	h->name_internal[0] = '@';
	for (i = 0; i < sizeof(h->name_internal) - 1; i++) {
		unsigned char c = toupper(h->name_internal[i]);
		h->name_internal[i] = c;
	}

	for (i = 0; i < sizeof(h->lc_names) / sizeof(h->name_en); i++) {
		if (names->localized[i] == NULL) {
			src = names->basic;
		} else {
			src = names->localized[i];
		}
		strncpy((char *)&h->lc_names[i], src, sizeof(h->lc_names[i]) - 1);
		strncpy((char *)&h->lc_lnames[i], src, sizeof(h->lc_lnames[i]) - 1);
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

    struct g3a_header *h = calloc(1, sizeof(struct g3a_header));
    assert(h != NULL);

    memcpy(h->magic, MAGIC, sizeof(MAGIC));
    h->_pad3[0] = 0x01;
    h->_pad3[1] = 0x01;	// Doesn't seem necessary, but we'll use it
    strncpy(h->version, "01.00.0000", sizeof(h->version) - 1);
    g3a_fillTimestamp(h);
    return h;
}

void g3a_fillSize(struct g3a_header *h, u32 codeSize) {
	u32 outSize;

    dump_u32(codeSize, &h->cksum2_ofs);
    outSize = codeSize + 0x7004;
    dump_u32(outSize, &h->size);
}
/*
 * Read inFile, checksum it, and write the contents into outFile.
 */
u32 g3a_processRaw(const char *inFile, FILE *outFile, u32 *size) {
    u32 cksum = 0;
    u8 buf[FREAD_CHUNK];
    size_t rsize;
	FILE *inFP;

    inFP = fopen(inFile, "rb");
    assert(inFP != NULL);

    *size = 0;
    do {
        // Read a chunk, update checksum, write
        rsize = fread(buf, 1, FREAD_CHUNK, inFP);
        cksum += checksum(buf, rsize);
        fwrite(buf, rsize, 1, outFile);
        *size += rsize;
    } while (rsize == FREAD_CHUNK);

    fclose(inFP);
    return cksum;
}
