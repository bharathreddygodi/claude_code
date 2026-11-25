/*
 * BIT-20: Parity and Gray code
 * Parity = 1 if the number of set bits is odd (UART parity bit, simple error check).
 *   fold with XOR: x ^= x >> 16; x ^= x >> 8; ... ; return x & 1   O(log n)
 * Gray code: consecutive values differ by one bit (rotary encoders, FIFO pointers
 * crossing clock domains).  gray = b ^ (b >> 1);  decode by XOR-ing prefixes.
 */
#include <stdio.h>
#include <stdint.h>

int parity(uint32_t x)
{
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (int)(x & 1);
}

uint32_t to_gray(uint32_t b) { return b ^ (b >> 1); }

uint32_t from_gray(uint32_t g)
{
    g ^= g >> 16;
    g ^= g >> 8;
    g ^= g >> 4;
    g ^= g >> 2;
    g ^= g >> 1;
    return g;
}

int main(void)
{
    uint32_t i;
    printf("parity(0x7)=%d parity(0xF)=%d parity(0x80000001)=%d\n",
           parity(0x7), parity(0xF), parity(0x80000001u));
    printf(" n  gray  decode\n");
    for (i = 0; i < 8; i++)
        printf("%2u  %u%u%u   %u %s\n", (unsigned)i, (unsigned)(to_gray(i) >> 2 & 1),
               (unsigned)(to_gray(i) >> 1 & 1), (unsigned)(to_gray(i) & 1),
               (unsigned)from_gray(to_gray(i)), from_gray(to_gray(i)) == i ? "PASS" : "FAIL");
    return 0;
}
