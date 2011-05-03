#include <string.h>

#include "config.h"

/*
 * Get file part of path, returning the beginning of basename or NULL if
 * path is a directory.
 */
const char *basename(const char *path)
{
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

/*
 * Endianness wrappers for dumping non-byte values
 */
void dump_u16(u16 v, u16 *loc) {
#if !IS_BIG_ENDIAN
    v = (v >> 8) | (v << 8 & 0xFF);
#endif
    *loc = v;
}
void dump_u32(u32 v, u32 *loc) {
#if !IS_BIG_ENDIAN
    // Convert to big-endian
    v = (v >> 24 & 0xFF) | (v >> 8 & 0xFF00)
        | (v << 8 & 0xFF0000) | (v << 24 & 0xFF000000);
#endif
    *loc = v;
}
