/*
 * TMR-18: Tickless idle - compute earliest deadline, program compare, sleep
 * Ticked idle: the 1 ms SysTick wakes the CPU 1000 times a second even with nothing to do.
 * Tickless: when idle,
 *   1) next = earliest timer deadline (sorted list head / heap top)
 *   2) sleep_ticks = min(next - now, max the hardware compare can reach)
 *   3) program the compare register (LPTIM/RTC), stop the tick, enter low-power sleep
 *   4) on wake (compare OR any other interrupt), read the real elapsed time from the
 *      free-running counter and advance the tick count by that much
 * Trade-offs: wake-up latency from deep sleep, timer resolution, lost accuracy if the
 * low-power clock drifts, and an early wake (e.g. UART IRQ) must still fix the tick count.
 * FreeRTOS: configUSE_TICKLESS_IDLE + portSUPPRESS_TICKS_AND_SLEEP().
 */
#include <stdio.h>
#include <stdint.h>

#define MAX_SLEEP 1000u               /* compare register range, in ticks */

static uint32_t deadlines[] = {30, 250, 260, 1900, 4000};   /* pending timer expiries (ms) */
#define ND (sizeof deadlines / sizeof deadlines[0])
static unsigned next_idx;

static int next_deadline(uint32_t *d) { if (next_idx >= ND) return 0; *d = deadlines[next_idx]; return 1; }

int main(void)
{
    uint32_t now = 0, d, wakeups_tickless = 0, wakeups_ticked = 4000;
    uint32_t early_irq_at = 1200;     /* a UART byte arrives while sleeping */

    printf("tickless run until t=4000 ms:\n");
    while (next_deadline(&d)) {
        uint32_t sleep = d - now;
        if (sleep > MAX_SLEEP) sleep = MAX_SLEEP;          /* compare can't reach further */
        /* LPTIM->CMP = LPTIM->CNT + sleep; stop SysTick; __WFI(); */
        uint32_t woke = now + sleep;
        const char *why = (woke == d) ? "timer due" : "compare limit, sleep again";
        if (now < early_irq_at && early_irq_at < woke) { woke = early_irq_at; why = "UART IRQ (early wake)"; }
        printf("  t=%4u sleep %4u ticks -> woke at %4u (%s)\n", (unsigned)now, (unsigned)(woke - now), (unsigned)woke, why);
        now = woke;                                        /* advance tick count by real elapsed time */
        wakeups_tickless++;
        if (now == d) next_idx++;                          /* run the expired timer */
        if (now == early_irq_at) early_irq_at = 0xFFFFFFFFu;
    }
    printf("wake-ups: ticked idle = %u, tickless = %u\n", (unsigned)wakeups_ticked, (unsigned)wakeups_tickless);
    return 0;
}
