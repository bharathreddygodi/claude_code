/*
 * STR-12: Hex string <-> bytes, plus a hex dump (very common in firmware/protocol work)
 *  "DEADBEEF" -> {0xDE,0xAD,0xBE,0xEF} and back
 *  validate characters, odd length, upper/lower case
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int hex_to_bytes(const char *hex, uint8_t *out, size_t cap)   /* returns byte count or -1 */
{
    size_t len = strlen(hex), i;
    if (len % 2 || len / 2 > cap) return -1;
    for (i = 0; i < len; i += 2) {
        int hi = hexval(hex[i]), lo = hexval(hex[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        out[i / 2] = (uint8_t)(hi << 4 | lo);
    }
    return (int)(len / 2);
}

void bytes_to_hex(const uint8_t *b, size_t n, char *out)      /* out needs 2n+1 */
{
    static const char d[] = "0123456789ABCDEF";
    size_t i;
    for (i = 0; i < n; i++) {
        out[2 * i] = d[b[i] >> 4];
        out[2 * i + 1] = d[b[i] & 0xF];
    }
    out[2 * n] = '\0';
}

void hex_dump(const void *data, size_t n)
{
    const uint8_t *p = data;
    size_t i, j;
    for (i = 0; i < n; i += 16) {
        printf("%04X  ", (unsigned)i);
        for (j = 0; j < 16; j++)
            j + i < n ? printf("%02X ", p[i + j]) : printf("   ");
        printf(" |");
        for (j = 0; j < 16 && i + j < n; j++)
            putchar(p[i + j] >= 32 && p[i + j] < 127 ? p[i + j] : '.');
        printf("|\n");
    }
}

int main(void)
{
    uint8_t buf[16];
    char back[40];
    int n = hex_to_bytes("DeadBeef0042", buf, sizeof buf);

    bytes_to_hex(buf, (size_t)n, back);
    printf("hex_to_bytes -> %d bytes, back to hex: %s\n", n, back);
    printf("invalid \"XYZ1\" -> %d, odd \"ABC\" -> %d\n", hex_to_bytes("XYZ1", buf, 16), hex_to_bytes("ABC", buf, 16));
    hex_dump("Hello, embedded interview!\x01\x02", 28);
    return 0;
}
