/*
 * Detect system endianness
 * Little endian: least significant byte at the lowest address (x86, most ARM).
 * Big endian:    most significant byte at the lowest address (network order).
 * Three common ways: pointer cast, union, compile-time macro.
 */
#include <stdio.h>
#include <stdint.h>

int is_little_endian_ptr(void)
{
    uint32_t x = 1;
    return *(uint8_t *)&x == 1;
}

int is_little_endian_union(void)
{
    union { uint32_t u32; uint8_t u8[4]; } u = { 0x01020304 };
    return u.u8[0] == 0x04;
}

int main(void)
{
    uint32_t v = 0x12345678;
    uint8_t *p = (uint8_t *)&v;
    int i;

    printf("pointer method: %s endian\n", is_little_endian_ptr() ? "Little" : "Big");
    printf("union method  : %s endian\n", is_little_endian_union() ? "Little" : "Big");
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    printf("compile time  : Little endian (__BYTE_ORDER__)\n");
#elif defined(__BYTE_ORDER__)
    printf("compile time  : Big endian (__BYTE_ORDER__)\n");
#endif
    printf("0x12345678 in memory: ");
    for (i = 0; i < 4; i++)
        printf("%02X ", p[i]);
    printf("\n");
    return 0;
}
