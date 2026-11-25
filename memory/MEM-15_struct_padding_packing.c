/*
 * MEM-15: Structure padding, alignment and packing
 * - Each member is aligned to its natural alignment; struct size is rounded up to
 *   the largest member alignment (so arrays of the struct stay aligned).
 * - Reorder members largest -> smallest to minimise padding.
 * - #pragma pack(1) / __attribute__((packed)) removes padding: smaller, but
 *   unaligned access is slower or even faults on some CPUs (Cortex-M0, old ARM).
 * - Bit-fields: compact flags, but layout is implementation-defined (avoid for HW regs
 *   or wire formats if portability matters).
 * Interview points: predict sizeof() by hand, why memcmp on structs is unsafe,
 * when packing is needed (protocol headers, file formats).
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

struct bad {             /* char(1)+pad(3) int(4) char(1)+pad(7) double(8) = 24 */
    char a;
    int b;
    char c;
    double d;
};

struct good {            /* double(8) int(4) char char + pad(2) = 16 */
    double d;
    int b;
    char a;
    char c;
};

#pragma pack(push, 1)
struct packed {          /* 1 + 4 + 1 + 8 = 14, no padding */
    char a;
    int b;
    char c;
    double d;
};
#pragma pack(pop)

struct flags {           /* bit-fields fit in one 32-bit unit */
    uint32_t enable : 1;
    uint32_t mode   : 3;
    uint32_t speed  : 4;
    uint32_t rsvd   : 24;
};

#define SHOW(T, m) printf("  %-8s offset %2u\n", #m, (unsigned)offsetof(T, m))

int main(void)
{
    printf("struct bad    sizeof = %u\n", (unsigned)sizeof(struct bad));
    SHOW(struct bad, a); SHOW(struct bad, b); SHOW(struct bad, c); SHOW(struct bad, d);

    printf("struct good   sizeof = %u (reordered)\n", (unsigned)sizeof(struct good));
    SHOW(struct good, d); SHOW(struct good, b); SHOW(struct good, a); SHOW(struct good, c);

    printf("struct packed sizeof = %u (pack(1))\n", (unsigned)sizeof(struct packed));
    SHOW(struct packed, a); SHOW(struct packed, b); SHOW(struct packed, c); SHOW(struct packed, d);

    printf("struct flags  sizeof = %u (bit-fields)\n", (unsigned)sizeof(struct flags));
    struct flags f = {1, 5, 9, 0};
    printf("  enable=%u mode=%u speed=%u\n", f.enable, f.mode, f.speed);

    /* Note: double alignment is 4 on 32-bit x86 Linux but 8 on Windows/ARM,
       so 'bad' may print 20 or 24 depending on ABI - good talking point. */
    return 0;
}
