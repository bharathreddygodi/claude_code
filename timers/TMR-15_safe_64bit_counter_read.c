/*
 * TMR-15: Safe 64-bit counter read from two 32-bit registers
 * Case 1 - hardware gives CNT_HI and CNT_LO (e.g. a 64-bit RTC/timer on a 32-bit bus):
 *     do { hi = CNT_HI; lo = CNT_LO; hi2 = CNT_HI; } while (hi != hi2);
 *   If lo wrapped between the reads, hi changed, so retry. No interrupt masking needed.
 *   (Some chips latch HI when LO is read - check the datasheet, then one read order works.)
 * Case 2 - 32-bit hardware counter + software overflow count incremented in the
 *   overflow ISR (common "extend a 16/32-bit timer" trick). Race: counter wrapped but
 *   the overflow ISR hasn't run yet. Fix: read with IRQs off and check the pending
 *   overflow flag; if pending and the counter value is small, add one more overflow.
 * Both are simulated; the "hardware" advances between register reads.
 */
#include <stdio.h>
#include <stdint.h>

/* ---------- case 1: two hardware registers ---------- */
#define START 0x00000001FFFFFFFFull       /* one tick before LO wraps */
static uint64_t hw_counter = START;
static int reads;
/* the counter keeps running: it advances 2 ticks during every register read */
static uint32_t CNT_LO(void) { uint32_t v = (uint32_t)hw_counter; hw_counter += 2; reads++; return v; }
static uint32_t CNT_HI(void) { uint32_t v = (uint32_t)(hw_counter >> 32); hw_counter += 2; reads++; return v; }

static uint64_t read64_naive(void)
{
    uint32_t hi = CNT_HI();
    uint32_t lo = CNT_LO();          /* LO wrapped after HI was read */
    return ((uint64_t)hi << 32) | lo;
}

static uint64_t read64_safe(void)
{
    uint32_t hi, lo, hi2;
    do {
        hi = CNT_HI();
        lo = CNT_LO();
        hi2 = CNT_HI();
    } while (hi != hi2);
    return ((uint64_t)hi << 32) | lo;
}

/* ---------- case 2: 32-bit HW counter + software overflow count ---------- */
static uint32_t TIM_CNT;             /* hardware counter */
static int TIM_OVF_PENDING;          /* overflow flag, ISR not run yet */
static uint32_t sw_overflows;        /* incremented by overflow ISR */

static uint64_t ext_read_naive(void)
{
    return ((uint64_t)sw_overflows << 32) | TIM_CNT;
}

static uint64_t ext_read_safe(void)
{
    uint32_t ovf, cnt;
    /* uint32_t s = __get_PRIMASK(); __disable_irq(); */
    ovf = sw_overflows;
    cnt = TIM_CNT;
    if (TIM_OVF_PENDING && cnt < 0x80000000u) ovf++;   /* wrapped, ISR still pending */
    /* __set_PRIMASK(s); */
    return ((uint64_t)ovf << 32) | cnt;
}

#define HI(x) ((unsigned)((x) >> 32))   /* this MinGW printf has no %llX */
#define LO(x) ((unsigned)(x))

int main(void)
{
    uint64_t v;

    v = read64_naive();
    printf("case 1 naive: 0x%08X%08X  -> WRONG (old hi + new lo, ~4 billion ticks behind)\n", HI(v), LO(v));
    hw_counter = START; reads = 0;
    v = read64_safe();
    printf("case 1 safe : 0x%08X%08X  -> consistent (%d register reads, one retry)\n", HI(v), LO(v), reads);

    sw_overflows = 1;                    /* counter just wrapped from 0xFFFFFFFF to 3 ... */
    TIM_CNT = 3;
    TIM_OVF_PENDING = 1;                 /* ... but the overflow ISR hasn't run yet */
    printf("case 2 naive: 0x%08X%08X  -> WRONG (went backwards)\n", HI(ext_read_naive()), LO(ext_read_naive()));
    printf("case 2 safe : 0x%08X%08X  -> correct (pending overflow counted)\n", HI(ext_read_safe()), LO(ext_read_safe()));
    return 0;
}
