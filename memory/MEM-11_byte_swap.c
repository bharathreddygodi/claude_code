/*
 * MEM-11: Convert endianness (byte swap) for 16/32/64-bit values,
 * plus portable "read big-endian bytes from a buffer" (used in protocol parsing).
 * Interview points: masks + shifts, use unsigned types, htonl/ntohl,
 * __builtin_bswap32 compiles to one instruction (BSWAP on x86, REV on ARM).
 */
#include <stdio.h>
#include <stdint.h>

uint16_t swap16(uint16_t x) { return (uint16_t)((x << 8) | (x >> 8)); }

uint32_t swap32(uint32_t x)
{
    return ((x & 0x000000FFu) << 24) |
           ((x & 0x0000FF00u) << 8)  |
           ((x & 0x00FF0000u) >> 8)  |
           ((x & 0xFF000000u) >> 24);
}

uint64_t swap64(uint64_t x)
{
    return ((uint64_t)swap32((uint32_t)x) << 32) | swap32((uint32_t)(x >> 32));
}

/* Endian-independent: gives the same result on any host */
uint32_t read_be32(const uint8_t *b)
{
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | b[3];
}

void write_le32(uint8_t *b, uint32_t v)
{
    b[0] = (uint8_t)v;
    b[1] = (uint8_t)(v >> 8);
    b[2] = (uint8_t)(v >> 16);
    b[3] = (uint8_t)(v >> 24);
}

int main(void)
{
    uint8_t buf[4] = {0x12, 0x34, 0x56, 0x78}, out[4];
    uint64_t s64 = swap64(0x0102030405060708ULL);

    printf("swap16(0x1234)     = 0x%04X %s\n", swap16(0x1234), swap16(0x1234) == 0x3412 ? "PASS" : "FAIL");
    printf("swap32(0x12345678) = 0x%08X %s\n", (unsigned)swap32(0x12345678),
           swap32(0x12345678) == 0x78563412u ? "PASS" : "FAIL");
    printf("swap64             = 0x%08X%08X %s\n", (unsigned)(s64 >> 32), (unsigned)s64,
           s64 == 0x0807060504030201ULL ? "PASS" : "FAIL");
    printf("read_be32          = 0x%08X %s\n", (unsigned)read_be32(buf),
           read_be32(buf) == 0x12345678u ? "PASS" : "FAIL");
    write_le32(out, 0x12345678);
    printf("write_le32         = %02X %02X %02X %02X %s\n", out[0], out[1], out[2], out[3],
           (out[0] == 0x78 && out[3] == 0x12) ? "PASS" : "FAIL");
#ifdef __GNUC__
    printf("__builtin_bswap32  = 0x%08X\n", (unsigned)__builtin_bswap32(0x12345678));
#endif
    return 0;
}
