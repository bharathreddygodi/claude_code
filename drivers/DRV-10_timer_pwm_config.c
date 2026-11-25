/*
 * DRV-10: Timer / PWM configuration math
 *   f_out = f_clk / ((PSC + 1) * (ARR + 1))
 *   duty  = CCR / (ARR + 1)
 * Pick the smallest prescaler that lets ARR fit in the timer width (16-bit -> 65535):
 * larger ARR = finer duty-cycle resolution. Resolution in bits = log2(ARR + 1).
 * Also: input capture to measure a frequency = f_timer / (capture2 - capture1),
 * with unsigned subtraction handling counter wrap-around.
 */
#include <stdio.h>
#include <stdint.h>

typedef struct { uint32_t psc, arr; double actual_hz; } timer_cfg_t;

int pwm_calc(uint32_t fclk, uint32_t f_out, uint32_t max_arr, timer_cfg_t *c)
{
    uint32_t ticks, psc;
    if (f_out == 0 || f_out > fclk) return -1;
    ticks = fclk / f_out;                       /* (PSC+1)*(ARR+1) */
    psc = (ticks + max_arr) / (max_arr + 1);    /* ceil(ticks / (max_arr+1)) */
    if (psc == 0) psc = 1;
    if (psc > 65536) return -1;                 /* can't reach that low a frequency */
    c->psc = psc - 1;
    c->arr = ticks / psc - 1;
    c->actual_hz = (double)fclk / ((c->psc + 1.0) * (c->arr + 1.0));
    return 0;
}

uint32_t duty_to_ccr(const timer_cfg_t *c, unsigned percent) { return (c->arr + 1) * percent / 100; }

static int bits_of(uint32_t n) { int b = 0; while (n > 1) { n >>= 1; b++; } return b; }

int main(void)
{
    uint32_t fclk = 72000000u, freqs[] = {50, 1000, 20000, 1};
    unsigned i;
    timer_cfg_t c;

    for (i = 0; i < 4; i++) {
        if (pwm_calc(fclk, freqs[i], 0xFFFF, &c) == 0)
            printf("%6u Hz: PSC=%5u ARR=%5u actual=%.3f Hz resolution=%d bits CCR(25%%)=%u\n",
                   (unsigned)freqs[i], (unsigned)c.psc, (unsigned)c.arr, c.actual_hz,
                   bits_of(c.arr + 1), (unsigned)duty_to_ccr(&c, 25));
        else
            printf("%6u Hz: not reachable with a 16-bit prescaler\n", (unsigned)freqs[i]);
    }

    {   /* input capture across a 16-bit counter wrap */
        uint16_t cap1 = 65000, cap2 = 1464;      /* counter wrapped between captures */
        uint16_t period = (uint16_t)(cap2 - cap1);
        uint32_t f_timer = 1000000u;             /* 1 MHz timer clock */
        printf("input capture: cap1=%u cap2=%u period=%u ticks -> %u Hz\n",
               cap1, cap2, period, (unsigned)(f_timer / period));
    }
    return 0;
}
