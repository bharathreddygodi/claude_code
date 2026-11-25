/*
 * BIT-13: Subsets via bitmask, and Gray code sequences
 * Subsets: for n items, every number 0 .. 2^n - 1 is a subset; bit i set = item i included.
 *   - all subsets: for (m = 0; m < (1u << n); m++)
 *   - all subsets OF a given mask (submask walk): for (s = mask; s; s = (s - 1) & mask)
 * Gray code: consecutive values differ in exactly one bit.
 *   g(i) = i ^ (i >> 1);  decode by XOR-ing all right shifts (see BIT-20).
 *   Uses: rotary encoders, FIFO pointers crossing clock domains, Karnaugh maps.
 */
#include <stdio.h>
#include <stdint.h>

static const char *items[] = {"LED", "UART", "SPI", "ADC"};

static void print_subset(uint32_t m, int n)
{
    int i, first = 1;
    printf("{");
    for (i = 0; i < n; i++)
        if (m & (1u << i)) { printf("%s%s", first ? "" : ",", items[i]); first = 0; }
    printf("}");
}

int main(void)
{
    int n = 3, count = 0, i, ok = 1;
    uint32_t m, s, mask = 0xB;      /* LED, UART, ADC */

    printf("all subsets of 3 items:\n ");
    for (m = 0; m < (1u << n); m++) { print_subset(m, n); putchar(' '); count++; }
    printf("\n count = %d (expect 2^3 = 8)\n", count);

    printf("submasks of 0x%X:\n ", (unsigned)mask);
    for (s = mask; s; s = (s - 1) & mask) { print_subset(s, 4); putchar(' '); }
    printf("{}\n");

    printf("4-bit Gray code:\n");
    for (i = 0; i < 16; i++) {
        uint32_t g = (uint32_t)i ^ ((uint32_t)i >> 1), prev = (uint32_t)(i - 1) ^ ((uint32_t)(i - 1) >> 1);
        if (i > 0) ok &= (__builtin_popcount(g ^ prev) == 1);
        printf(" %2d -> %u%u%u%u%s", i, (unsigned)(g >> 3 & 1), (unsigned)(g >> 2 & 1),
               (unsigned)(g >> 1 & 1), (unsigned)(g & 1), (i % 4 == 3) ? "\n" : "");
    }
    printf("each step changes exactly one bit -> %s\n", ok ? "PASS" : "FAIL");
    return 0;
}
