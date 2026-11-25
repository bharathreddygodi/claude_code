/*
 * BIT-07: Reverse the bits of a 32-bit integer (and of a byte)
 * 1) loop: shift result left, take x's LSB          O(32)
 * 2) divide & conquer: swap halves, then bytes, nibbles, pairs, bits   O(log n)
 * 3) byte lookup table (fast on MCUs, 256 bytes of flash)
 * ARM has RBIT instruction; __builtin_bitreverse32 exists on clang only.
 */
#include <stdio.h>
#include <stdint.h>

uint32_t rev_loop(uint32_t x)
{
    uint32_t r = 0;
    int i;
    for (i = 0; i < 32; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

uint32_t rev_swap(uint32_t x)
{
    x = (x >> 16) | (x << 16);
    x = ((x & 0xFF00FF00u) >> 8) | ((x & 0x00FF00FFu) << 8);
    x = ((x & 0xF0F0F0F0u) >> 4) | ((x & 0x0F0F0F0Fu) << 4);
    x = ((x & 0xCCCCCCCCu) >> 2) | ((x & 0x33333333u) << 2);
    x = ((x & 0xAAAAAAAAu) >> 1) | ((x & 0x55555555u) << 1);
    return x;
}

uint8_t rev_byte(uint8_t b)
{
    b = (uint8_t)((b & 0xF0) >> 4 | (b & 0x0F) << 4);
    b = (uint8_t)((b & 0xCC) >> 2 | (b & 0x33) << 2);
    b = (uint8_t)((b & 0xAA) >> 1 | (b & 0x55) << 1);
    return b;
}

int main(void)
{
    uint32_t t[] = {1, 0x80000000u, 0x0000000Fu, 0x12345678u, 0xF0F0F0F0u};
    unsigned i;
    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        uint32_t a = rev_loop(t[i]), b = rev_swap(t[i]);
        printf("0x%08X -> 0x%08X 0x%08X %s\n", (unsigned)t[i], (unsigned)a, (unsigned)b, a == b ? "PASS" : "FAIL");
    }
    printf("rev_byte(0x01)=0x%02X rev_byte(0xB1)=0x%02X\n", rev_byte(0x01), rev_byte(0xB1));
    return 0;
}
