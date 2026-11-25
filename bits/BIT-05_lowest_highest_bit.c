/*
 * BIT-05: Find lowest / highest set bit
 *  x & -x            isolates the lowest set bit
 *  x & (x - 1)       clears the lowest set bit
 *  position of lowest set bit  (count trailing zeros, __builtin_ctz)
 *  position of highest set bit (floor(log2 x), 31 - __builtin_clz)
 * Used for: priority schedulers (RTOS ready bitmap), interrupt pending registers.
 * Note: __builtin_ctz/clz(0) is undefined -> check for 0 first.
 */
#include <stdio.h>
#include <stdint.h>

int lowest_bit_pos(uint32_t x)
{
    int pos = 0;
    if (!x) return -1;
    while (!(x & 1)) { x >>= 1; pos++; }
    return pos;
}

int highest_bit_pos(uint32_t x)          /* binary search, O(log 32) */
{
    int pos = 0;
    if (!x) return -1;
    if (x >= 1u << 16) { x >>= 16; pos += 16; }
    if (x >= 1u << 8)  { x >>= 8;  pos += 8; }
    if (x >= 1u << 4)  { x >>= 4;  pos += 4; }
    if (x >= 1u << 2)  { x >>= 2;  pos += 2; }
    if (x >= 1u << 1)  { pos += 1; }
    return pos;
}

int main(void)
{
    uint32_t t[] = {1, 12, 0x80, 0x00F00000u, 0x80000001u};
    unsigned i;
    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        uint32_t x = t[i];
        printf("x=0x%08X lowbit=0x%08X clear_low=0x%08X lowpos=%2d(%2d) highpos=%2d(%2d)\n",
               (unsigned)x, (unsigned)(x & (0u - x)), (unsigned)(x & (x - 1)),
               lowest_bit_pos(x), __builtin_ctz(x), highest_bit_pos(x), 31 - __builtin_clz(x));
    }
    printf("zero -> lowpos=%d highpos=%d\n", lowest_bit_pos(0), highest_bit_pos(0));
    return 0;
}
