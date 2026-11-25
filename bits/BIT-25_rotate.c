/*
 * BIT-25: Rotate left / right (circular shift)
 * rotl(x, n) = (x << n) | (x >> (32 - n))
 * Gotcha: n == 0 makes (x >> 32) UB -> mask with (-n & 31). Compilers turn
 * this exact pattern into a single ROL/ROR instruction.
 * Also: right-shifting a NEGATIVE signed int is implementation-defined
 * (usually arithmetic) -> always rotate/shift unsigned types.
 */
#include <stdio.h>
#include <stdint.h>

uint32_t rotl32(uint32_t x, unsigned n)
{
    n &= 31;
    return (x << n) | (x >> ((32 - n) & 31));
}

uint32_t rotr32(uint32_t x, unsigned n)
{
    n &= 31;
    return (x >> n) | (x << ((32 - n) & 31));
}

uint8_t rotl8(uint8_t x, unsigned n)
{
    n &= 7;
    return (uint8_t)((x << n) | (x >> ((8 - n) & 7)));
}

int main(void)
{
    uint32_t x = 0x12345678u;
    printf("rotl32(0x%08X, 8)  = 0x%08X (expect 0x34567812)\n", (unsigned)x, (unsigned)rotl32(x, 8));
    printf("rotr32(0x%08X, 8)  = 0x%08X (expect 0x78123456)\n", (unsigned)x, (unsigned)rotr32(x, 8));
    printf("rotl32(x, 0)        = 0x%08X (no UB)\n", (unsigned)rotl32(x, 0));
    printf("rotl32(x, 36)       = 0x%08X (same as 4)\n", (unsigned)rotl32(x, 36));
    printf("rotl8(0x81, 1)      = 0x%02X (expect 0x03)\n", rotl8(0x81, 1));
    printf("-16 >> 2 (signed)   = %d (implementation-defined!)\n", -16 >> 2);
    return 0;
}
