/*
 * STR-11: Run-length encoding / decoding (string compression)
 * "aaabccddd" -> "a3b1c2d3". Variant (CTCI): return original if not shorter.
 * Interview points: output buffer size (worst case 2x+ for counts >= 10),
 * multi-digit counts, bounds checking with snprintf.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int rle_encode(const char *in, char *out, size_t cap)   /* returns length or -1 */
{
    size_t w = 0;
    while (*in) {
        char c = *in;
        int run = 0, n;
        while (*in == c) { in++; run++; }
        n = snprintf(out + w, cap - w, "%c%d", c, run);
        if (n < 0 || (size_t)n >= cap - w) return -1;
        w += (size_t)n;
    }
    out[w] = '\0';
    return (int)w;
}

int rle_decode(const char *in, char *out, size_t cap)
{
    size_t w = 0;
    while (*in) {
        char c = *in++;
        int run = 0;
        while (isdigit((unsigned char)*in)) run = run * 10 + (*in++ - '0');
        while (run--) {
            if (w + 1 >= cap) return -1;
            out[w++] = c;
        }
    }
    out[w] = '\0';
    return (int)w;
}

int main(void)
{
    const char *src = "aaabccddddddddddddx";
    char enc[64], dec[64];

    rle_encode(src, enc, sizeof enc);
    rle_decode(enc, dec, sizeof dec);
    printf("encode: %s -> %s\n", src, enc);
    printf("decode: %s -> %s %s\n", enc, dec, strcmp(dec, src) ? "FAIL" : "PASS");
    printf("tiny buffer -> %d (expect -1)\n", rle_encode(src, enc, 4));
    return 0;
}
