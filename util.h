#include <stdlib.h>

char *basename(const char *path);
u32 checksum(const void *ptr, size_t bytes);
void dump_u16(u16 v, u16 *loc);
void dump_u32(u32 v, u32 *loc);
void *mallocs(size_t size);
void *callocs(size_t count, size_t size);
