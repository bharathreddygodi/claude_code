/*
 * CON-18: Critical sections and torn reads  (also covers CON-19: torn 64-bit read)
 * 1) Nest-safe critical section: save the interrupt state, disable, restore the SAVED
 *    state (not blindly enable!) - otherwise an inner exit re-enables too early.
 *      Cortex-M:  uint32_t s = __get_PRIMASK(); __disable_irq(); ... __set_PRIMASK(s);
 * 2) Torn read: a 64-bit tick counter on a 32-bit CPU is read in two halves; an ISR
 *    incrementing between them gives a garbage value. Fixes: critical section, or
 *    lock-free "read high, read low, re-read high, retry if high changed".
 * Interrupt masking is simulated with a flag; the "ISR" is injected between reads.
 */
#include <stdio.h>
#include <stdint.h>

static volatile uint32_t PRIMASK;          /* 1 = interrupts disabled */
static uint32_t enter_critical(void) { uint32_t s = PRIMASK; PRIMASK = 1; return s; }
static void exit_critical(uint32_t s)  { PRIMASK = s; }

static void inner(void)
{
    uint32_t s = enter_critical();
    printf("    inner: PRIMASK=%u\n", (unsigned)PRIMASK);
    exit_critical(s);                      /* restores 1, NOT 0 -> outer still protected */
}

static void outer(void)
{
    uint32_t s = enter_critical();
    inner();
    printf("  outer after inner: PRIMASK=%u (still disabled -> correct)\n", (unsigned)PRIMASK);
    exit_critical(s);
}

/* ---- 64-bit counter split into two 32-bit words (as a 32-bit CPU sees it) ---- */
static volatile uint32_t tick_hi, tick_lo;
static void tick_isr(void) { if (++tick_lo == 0) tick_hi++; }

static int inject_at;                       /* simulate ISR firing between two reads */
static uint32_t read_lo(void) { uint32_t v = tick_lo; if (inject_at == 1) { tick_isr(); inject_at = 0; } return v; }
static uint32_t read_hi(void) { return tick_hi; }

static uint64_t read_naive(void)
{
    uint32_t lo = read_lo();                /* ISR fires right after this read */
    uint32_t hi = read_hi();
    return ((uint64_t)hi << 32) | lo;
}

static uint64_t read_safe(void)
{
    uint32_t hi, lo, hi2;
    do {
        hi = read_hi();
        lo = read_lo();
        hi2 = read_hi();
    } while (hi != hi2);                    /* high word changed -> lo may be stale, retry */
    return ((uint64_t)hi << 32) | lo;
}

int main(void)
{
    uint64_t v;
    printf("nested critical sections:\n");
    outer();
    printf("  after outer: PRIMASK=%u\n\n", (unsigned)PRIMASK);

    tick_hi = 0; tick_lo = 0xFFFFFFFFu; inject_at = 1;
    v = read_naive();
    printf("naive read : 0x%08X%08X  (true value is now 0x0000000100000000) -> TORN\n",
           (unsigned)(v >> 32), (unsigned)v);

    tick_hi = 0; tick_lo = 0xFFFFFFFFu; inject_at = 1;
    v = read_safe();
    printf("safe read  : 0x%08X%08X  -> consistent\n", (unsigned)(v >> 32), (unsigned)v);
    return 0;
}
