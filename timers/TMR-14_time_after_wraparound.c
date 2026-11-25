/*
 * TMR-14: time_after() with wraparound (Linux jiffies style)
 *   #define time_after(a, b)   ((int32_t)((b) - (a)) < 0)     a is later than b
 *   #define time_before(a, b)  time_after(b, a)
 * Why the signed cast works: unsigned subtraction is modulo 2^32, so (a - b) is the
 * real forward distance from b to a even across a wrap. Interpreting that distance as
 * signed means "less than half the ring ahead" = positive = later.
 * Where it breaks: if the two times are 2^31 or more ticks apart, the sign flips and
 * the answer is wrong. At 1 kHz, 2^31 ms = 24.9 days. So: never compare times further
 * apart than that (keep a 64-bit time for long spans), and never use a plain  a > b.
 * (Casting an out-of-range unsigned to int32_t is implementation-defined in C,
 *  but two's-complement wrap on every real compiler; C23 makes it defined.)
 */
#include <stdio.h>
#include <stdint.h>

#define time_after(a, b)     ((int32_t)((uint32_t)(b) - (uint32_t)(a)) < 0)
#define time_before(a, b)    time_after(b, a)
#define time_after_eq(a, b)  ((int32_t)((uint32_t)(a) - (uint32_t)(b)) >= 0)

static int naive_after(uint32_t a, uint32_t b) { return a > b; }

static void show(const char *what, uint32_t a, uint32_t b)
{
    printf("%-34s a=0x%08X b=0x%08X  naive a>b=%d  time_after(a,b)=%d  time_after(b,a)=%d\n",
           what, (unsigned)a, (unsigned)b, naive_after(a, b), time_after(a, b), time_after(b, a));
}

int main(void)
{
    uint32_t now = 0xFFFFFFF0u;
    uint32_t deadline = now + 0x20;          /* wraps to 0x00000010 */

    show("normal: 200 is after 100", 200, 100);
    show("deadline wrapped past 0", deadline, now);
    show("now (after wrap) vs old deadline", 0x00000015u, deadline);
    show("2^31 - 1 apart (still correct)", 0x7FFFFFFFu, 0u);
    show("2^31 apart (BREAKS: both say 1)", 0x80000000u, 0u);
    show("2^31 + 5 apart (wrong answer)", 0x80000005u, 0u);

    printf("\ntime_after_eq(now, deadline) loop near the wrap:\n");
    for (now = 0xFFFFFFFDu; now != 0x00000013u; now++)
        if (time_after_eq(now, deadline)) { printf("  expired at now=0x%08X (deadline 0x%08X) -> PASS\n",
                                                   (unsigned)now, (unsigned)deadline); break; }
    printf("max safe span at 1 kHz tick = %.1f days\n", 2147483648.0 / 1000 / 86400);
    return 0;
}
