#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

static __inline u32 u32_flip(u32 v);
static __inline u16 u16_flip(u16 v);

/*
 * Get file part of path, returning the beginning of basename or NULL if
 * path is a directory.
 */
const char *basename(const char *path) {
    const char *n = strrchr(path, '/');
    if (n == NULL)
        return path;
    else if (strlen(n) == 0)
        return NULL;
    return n;
}

/*
 * Simple checksum of bytes bytes at ptr
 */
u32 checksum(const void *ptr, size_t bytes) {
    unsigned int i;
    u8 *p = (u8 *)ptr;
    u32 sum = 0;

    for (i = 0; i < bytes; i++)
        sum += *p++;
    return sum;
}

// Flip endianness of v
static __inline u32 u32_flip(u32 v) {
    return (v >> 24 & 0xFF) | (v >> 8 & 0xFF00) | (v << 8 & 0xFF0000) |
           (v << 24 & 0xFF000000);
}
static __inline u16 u16_flip(u16 v) { return (v >> 8) | (v << 8 & 0xFF); }

// Native byte order to big-endian
u32 u32_ntobe(u32 v) {
#if !IS_BIG_ENDIAN
    return u32_flip(v);
#else
    return v;
#endif
}
u16 u16_ntobe(u16 v) {
#if !IS_BIG_ENDIAN
    return u16_flip(v);
#else
    return v;
#endif
}
// Native byte order to little-endian
u32 u32_ntole(u32 v) {
#if IS_BIG_ENDIAN
    return u32_flip(v);
#else
    return v;
#endif
}
u16 u16_ntole(u16 v) {
#if IS_BIG_ENDIAN
    return u16_flip(v);
#else
    return v;
#endif
}

/*
 * Endianness wrappers for dumping non-byte values as big-endian
 */
void dumpb_u16(u16 v, u16 *loc) { *loc = u16_ntobe(v); }
void dumpb_u32(u32 v, u32 *loc) { *loc = u32_ntobe(v); }

/*
 * 'safe' wrappers for malloc and friends
 */
void *mallocs(size_t size) {
    void *r = malloc(size);
    if (r == NULL) {
        printf("Failed to allocate memory; aborting.\n");
        exit(2);
    }
    return r;
}
void *callocs(size_t count, size_t size) {
    void *r = calloc(count, size);
    if (r == NULL) {
        printf("Failed to allocate memory; aborting.\n");
        exit(2);
    }
    return r;
}
