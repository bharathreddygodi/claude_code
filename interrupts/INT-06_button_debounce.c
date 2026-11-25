/*
 * INT-06: Button debouncing
 * Mechanical contacts bounce for ~1-20 ms -> one press looks like many edges.
 * Don't debounce with delays inside an edge ISR. Two good approaches:
 *  1) Periodic sampling (1 ms timer tick): accept a new state only after it has been
 *     stable for N consecutive samples.  <- shown, counter method
 *  2) Shift-register method: state = (state << 1) | pin; pressed when state == 0x..FF
 *  (Hybrid: edge IRQ starts a timer, disable the IRQ until the timer confirms.)
 */
#include <stdio.h>
#include <stdint.h>

#define STABLE_MS 5

typedef struct { uint8_t stable, last_raw, count; } debounce_t;

/* call every 1 ms; returns +1 on press, -1 on release, 0 otherwise */
int debounce_tick(debounce_t *d, uint8_t raw)
{
    if (raw != d->last_raw) { d->last_raw = raw; d->count = 0; return 0; }   /* still bouncing */
    if (raw == d->stable) return 0;
    if (++d->count >= STABLE_MS) {
        d->stable = raw;
        d->count = 0;
        return raw ? +1 : -1;
    }
    return 0;
}

int shift_debounce(uint8_t *hist, uint8_t raw)   /* returns 1 once per press */
{
    *hist = (uint8_t)((*hist << 1) | raw);
    return *hist == 0x7F;                         /* 0 then seven 1s: clean rising edge */
}

int main(void)
{
    /* 1 = pressed. Bouncy press around t=3..8, held, bouncy release around t=20..24 */
    const char *signal = "000101101111111111110100100000000";
    debounce_t d = {0, 0, 0};
    uint8_t hist = 0;
    int t, raw_edges = 0, presses = 0, shift_presses = 0;
    char prev = '0';

    for (t = 0; signal[t]; t++) {
        uint8_t raw = (uint8_t)(signal[t] - '0');
        int ev;
        if (signal[t] == '1' && prev == '0') raw_edges++;
        prev = signal[t];
        ev = debounce_tick(&d, raw);
        if (ev == 1)  { presses++; printf("t=%2d ms: PRESS\n", t); }
        if (ev == -1) printf("t=%2d ms: RELEASE\n", t);
        shift_presses += shift_debounce(&hist, raw);
    }
    printf("raw rising edges = %d, debounced presses = %d, shift method presses = %d\n",
           raw_edges, presses, shift_presses);
    return 0;
}
