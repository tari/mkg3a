#ifndef _G3A_H
#define _G3A_H

#include "config.h"
#include <stdio.h>

#define G3A_ICON_HEIGHT (64)
#define G3A_ICON_WIDTH (92)
#define G3A_ICON_MONO_HEIGHT (24)
#define G3A_ICON_MONO_WIDTH (64)

struct icons {
    u16 unselected[G3A_ICON_HEIGHT * G3A_ICON_WIDTH];
    u16 selected[G3A_ICON_HEIGHT * G3A_ICON_WIDTH];
    u16 mono[G3A_ICON_HEIGHT * G3A_ICON_WIDTH];
};

/* Shortcuts to create localized name members */
#define NAME(lc) char name_##lc[0x18]
#define LNAME(lc) char lname_##lc[0x24]

/* Need struct packing turned off. */
#pragma pack(1)
struct g3a_header {
    /* 0x0000 Magic nubmers */
    u8 magic[14];
    /* 0x000E Copy protection */
    u8 cprot[7];
    u8 _pad1;
    /* 0x0016 CRC, mostly unknown.  Leave 0 to ignore. */
    u16 crc;
    u8 _pad2[8];
    /* 0x0020 Simple checksum of entire file ignoring this and copy at end*/
    u32 cksum;
    u8 _pad3[0xA];
    /* 0x002E Offset of copy of cksum from content begin
     * (0x1F8 is file start + 0x71F8 for example) */
    u32 cksum2_ofs;
    u8 _pad4[0xE];
    /* 0x0040 Short name of add-in */
    char name_basic[0x1C];
    /* 0x005C Size of entire file */
    u32 size;
    /* 0x0060 Internal name, all-caps with @, like @CONV */
    char name_internal[0xB];
    /* 0x006B - 0x012F Localized names */
    union {
        char lc_names[8][0x18];
        struct {
            NAME(en);
            NAME(es);
            NAME(de);
            NAME(fr);
            NAME(pt);
            NAME(zh);
            NAME(un1); // Unknown (english?)
            NAME(un2); // Unknown (english?)
        };
    };
    u8 _pad5[5];
    /* 0x130 Version string */
    char version[0xC];
    /* 0x13C Creation/modification timestamp YYYY.MMDD.HHMM */
    char timestamp[15];
    u8 _pad6[37];
    /* 0x0170 - 0x028F More localized names */
    union {
        char lc_lnames[8][0x24];
        struct {
            LNAME(en);
            LNAME(es);
            LNAME(de);
            LNAME(fr);
            LNAME(pt);
            LNAME(zh);
            LNAME(un1); // Unknown (english?)
            LNAME(un2); // Unknown (english?)
        };
    };
    /* 0x0290 Monochrome icon- 64x24, 1 nibble per pixel, 0 or 7 */
    u8 icon_mono[64 * 24 / 2];
    u8 _pad7[0x92C];
    /* 0x0EBC File name (including .g3a extension) */
    char filename[0x144];
    /* 0x1000 Unselected icon (16 bpp 5-6-5, 92x64 pixels) */
    u16 icon_unsel[92 * 64];
    u8 _pad8[0x200];
    /* 0x4000 Selected icon */
    u16 icon_sel[92 * 64];
    u8 _pad9[0x200];
    /* 0x7000 Executable code */
};
#pragma pack()

struct lc_names {
    union {
        char *raw[10];
        struct {
            char *basic;
            char *internal;
            union {
                char *localized[8];
                struct {
                    char *en;
                    char *es;
                    char *de;
                    char *fr;
                    char *pt;
                    char *zh;
                    char *un1;
                    char *un2;
                };
            };
        };
    };
};

/* Creating bits of the file */
int g3a_mkG3A(const char *inFile, const char *outFile, struct lc_names *names,
              struct icons *icons, const char *version);
struct g3a_header *g3a_mkHeader(int type);
int g3a_processRaw(const char *inFile, FILE *outFile, u32 *size, u32 *cksum);

/* Processing bits of the file */
void g3a_fillCProt(struct g3a_header *h);
void g3a_fillIcons(struct g3a_header *h, struct icons *icons);
void g3a_fillNames(struct g3a_header *h, struct lc_names *names);
void g3a_fillSize(struct g3a_header *h, u32 codeSize);
void g3a_fillTimestamp(struct g3a_header *h);
void g3a_fillVersion(struct g3a_header *h, const char *version);

#endif /* _G3A_H */
