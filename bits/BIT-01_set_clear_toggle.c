/*
 * BIT-01: Set, clear, toggle, check a bit + read/modify/write a register field
 * Interview points: use 1U (unsigned) not 1, wrap macro args in (), never shift
 * by >= width (UB), read-modify-write on HW registers is not atomic (ISR race).
 */
#include <stdio.h>
#include <stdint.h>

#define BIT(n)              (1UL << (n))
#define SET_BIT(x, n)       ((x) |=  BIT(n))
#define CLEAR_BIT(x, n)     ((x) &= ~BIT(n))
#define TOGGLE_BIT(x, n)    ((x) ^=  BIT(n))
#define CHECK_BIT(x, n)     (((x) >> (n)) & 1UL)

/* Field helpers: mask of 'width' bits starting at 'pos' */
#define FIELD_MASK(pos, width)   (((1UL << (width)) - 1UL) << (pos))
#define FIELD_GET(x, pos, width) (((x) & FIELD_MASK(pos, width)) >> (pos))
#define FIELD_SET(x, pos, width, v) \
    ((x) = ((x) & ~FIELD_MASK(pos, width)) | (((uint32_t)(v) << (pos)) & FIELD_MASK(pos, width)))

static void print_bin(const char *tag, uint32_t x)
{
    int i;
    printf("%-14s 0x%08X  ", tag, (unsigned)x);
    for (i = 15; i >= 0; i--) {
        putchar(((x >> i) & 1) ? '1' : '0');
        if (i % 4 == 0) putchar(' ');
    }
    putchar('\n');
}

int main(void)
{
    uint32_t reg = 0;

    SET_BIT(reg, 3);       print_bin("set bit 3", reg);
    SET_BIT(reg, 7);       print_bin("set bit 7", reg);
    TOGGLE_BIT(reg, 0);    print_bin("toggle bit 0", reg);
    CLEAR_BIT(reg, 3);     print_bin("clear bit 3", reg);
    printf("check bit 7 = %lu, bit 3 = %lu\n", CHECK_BIT(reg, 7), CHECK_BIT(reg, 3));

    /* e.g. GPIO MODER-style: 2-bit mode field for pin 5 at bits [11:10] */
    FIELD_SET(reg, 10, 2, 2);  print_bin("field[11:10]=2", reg);
    printf("field[11:10] read back = %lu -> %s\n", FIELD_GET(reg, 10, 2),
           FIELD_GET(reg, 10, 2) == 2 ? "PASS" : "FAIL");
    return 0;
}
