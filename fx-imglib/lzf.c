#include <stddef.h>
#include <stdint.h>
/*
 * LZF is very easy to decompress.  Masking the top three bits of the
 * first byte in a block allows you to determine what sort of data there
 * is.
 *  000LLLLL                    Literal string of L+1 bytes
 *  LLLaaaaa bbbbbbbb           Backref of L+3 bytes
 *  111aaaaa LLLLLLLL bbbbbbbb  Backref of L+9 bytes
 * Starting address of backrefs is always at $ - (a << 8) - b - 1
 */

/* Additional options for tweaking:
 * Should test on some real-world images to see what sorts of backrefs we see.
 * If some backref fields are mostly unused, perhaps the L field could be expanded.
 */

/*
 * Decompress a LZF-compressed block of sz bytes when decompressed, reading
 * data from src and writing it to dst.
 */
void lzf_decompress(uint8_t restrict *dst, const uint8_t restrict *src, size_t sz) {
    int l;
    const uint8_t restrict *back;

    while (sz > 0) {
        // Get size and source for literal data
        uint8_t head = *src++;

        if (head >> 5 == 0) {       // Literal
            l = 1 + (head & 0x1F);
            back = src;
        } else {
            uint8_t a, b;
            back = src - 1;

            if (head >> 5 == 7)     // Long backref
                l = *src++ + 9;
            else                    // Short backref
                l = (head >> 5) + 3;

            a = head & 0x1f;
            b = *src++;
            back -= (a << 8) | b;
        }

        sz -= l;
        memcpy(dst, back, l);
        dst += l;
    }
}

