/*
 * BIT-24: Sign-extend an N-bit value to 32 bits
 * Very common in drivers: 12-bit ADC (two's complement), 24-bit audio samples,
 * 10-bit accelerometer axes, 20-bit pressure readings packed into registers.
 * Methods:
 *   1) XOR-subtract (portable, no UB):  m = 1 << (n-1);  r = (x ^ m) - m
 *   2) shift left then arithmetic shift right:  (int32_t)(x << (32-n)) >> (32-n)
 *      -> right shift of a negative int is implementation-defined (works on gcc/ARM)
 *   3) bit-field struct: struct { int32_t v : N; } s; s.v = x;  (compiler does it)
 * Always mask x to N bits first if the upper bits might contain garbage.
 */
#include <stdio.h>
#include <stdint.h>

int32_t sign_extend_xor(uint32_t x, unsigned n)          /* 1 <= n <= 32 */
{
    uint32_t m = 1u << (n - 1);
    if (n < 32) x &= (1u << n) - 1u;                       /* drop garbage above bit n-1 */
    return (int32_t)((x ^ m) - m);
}

int32_t sign_extend_shift(uint32_t x, unsigned n)
{
    unsigned s = 32 - n;
    return (int32_t)(x << s) >> s;                         /* implementation-defined >> */
}

static int32_t sign_extend_12_bitfield(uint32_t x)
{
    struct { int32_t v : 12; } s;
    s.v = (int32_t)(x & 0xFFF) - ((x & 0x800) ? 0x1000 : 0);   /* assign in range, no overflow */
    return s.v;
}

int main(void)
{
    struct { uint32_t raw; unsigned bits; int32_t expect; } t[] = {
        {0x7FF, 12, 2047},   {0x800, 12, -2048},  {0xFFF, 12, -1},  {0x001, 12, 1},
        {0x3FF, 10, -1},     {0x200, 10, -512},   {0x800000, 24, -8388608},
        {0x7FFFFF, 24, 8388607}, {0xF0F80, 12, -128},   /* garbage above bit 11 */
    };
    unsigned i;
    int ok = 1;

    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        int32_t a = sign_extend_xor(t[i].raw, t[i].bits);
        int32_t b = sign_extend_shift(t[i].raw, t[i].bits);
        int pass = (a == t[i].expect) && (b == t[i].expect);
        ok &= pass;
        printf("raw 0x%06X as %2u-bit -> xor %9d shift %9d %s\n", (unsigned)t[i].raw, t[i].bits, (int)a, (int)b,
               pass ? "PASS" : "FAIL");
    }
    printf("bitfield 12-bit 0xF80 -> %d\n", (int)sign_extend_12_bitfield(0xF80));
    printf("all -> %s\n", ok ? "PASS" : "FAIL");
    return 0;
}
