/*
 * TMR-16: delay_ms() and timeout loops that survive tick-counter wraparound
 * RIGHT:  start = now(); while ((uint32_t)(now() - start) < ms) { }
 *   Elapsed time is computed with unsigned subtraction, which is correct across a wrap.
 * WRONG:  end = now() + ms; while (now() < end) { }
 *   If end wraps past 0, the loop exits immediately (or hangs forever in other cases).
 * Timeout pattern for polling hardware: return an error instead of hanging.
 * Notes: delay_ms(1) may wait 0..1 ms of real time (tick granularity) -> wait ms+1
 * if you need "at least"; busy-wait burns power -> in an RTOS use vTaskDelay.
 * The tick counter is simulated: each call to now() advances time by 1 ms.
 */
#include <stdio.h>
#include <stdint.h>

static volatile uint32_t g_ms;
static uint32_t now(void) { return g_ms++; }      /* SysTick increments this every 1 ms */

static uint32_t delay_ms_good(uint32_t ms)       /* returns real ticks waited */
{
    uint32_t start = now(), t;
    while ((uint32_t)((t = now()) - start) < ms) { }
    return t - start;
}

static uint32_t delay_ms_bad(uint32_t ms)
{
    uint32_t start = now(), end = start + ms, t;
    while ((t = now()) < end) { if (t - start > 1000) return 9999; }   /* guard for the demo */
    return t - start;
}

/* poll a status bit with a timeout */
static volatile uint32_t STATUS;
static int wait_ready(uint32_t timeout_ms)
{
    uint32_t start = now();
    while (!(STATUS & 1u)) {
        if ((uint32_t)(now() - start) >= timeout_ms) return -1;   /* timed out */
        if (g_ms == 0x00000005u) STATUS = 1;     /* simulated: device becomes ready */
    }
    return 0;
}

int main(void)
{
    g_ms = 1000;
    printf("normal      : good waited %u ms, bad waited %u ms\n", (unsigned)delay_ms_good(50), (unsigned)delay_ms_bad(50));

    g_ms = 0xFFFFFFF0u;                          /* 16 ms before the wrap */
    printf("across wrap : good waited %u ms\n", (unsigned)delay_ms_good(50));
    g_ms = 0xFFFFFFF0u;
    printf("across wrap : bad  waited %u ms  <- exited at once, end wrapped to a small number\n",
           (unsigned)delay_ms_bad(50));

    g_ms = 0xFFFFFFF0u; STATUS = 0;
    printf("wait_ready across wrap, device ready at t=5 -> %d (0 = ready)\n", wait_ready(100));
    g_ms = 100; STATUS = 0;
    printf("wait_ready, device never ready            -> %d (-1 = timeout, no hang)\n", wait_ready(20));
    return 0;
}
