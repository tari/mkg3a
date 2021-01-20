#include <stdlib.h>

#include "config.h"

char *basename(const char *path);
u32 checksum(const void *ptr, size_t bytes);
void *mallocs(size_t size);
void *callocs(size_t count, size_t size);

void dumpb_u16(u16 v, u16 *loc);
void dumpb_u32(u32 v, u32 *loc);
u32 u32_ntobe(u32 v);
u16 u16_ntobe(u16 v);
u32 u32_ntole(u32 v);
u16 u16_ntole(u16 v);

// Just alias the functions since they only flip endianness
#define u32_beton u32_ntobe
#define u16_beton u16_ntobe
#define u32_leton u32_ntole
#define u16_leton u16_ntole
