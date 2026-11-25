/*
 * TMR-19: Button debounce with an edge interrupt + one-shot timer + state machine
 * (INT-06 shows the "sample every 1 ms" method; this is the interrupt-driven version,
 *  which lets the CPU sleep between presses.)
 *   IDLE --edge IRQ--> disable pin IRQ, start 20 ms timer --> WAIT_PRESS
 *   WAIT_PRESS --timer: pin still pressed?--> PRESSED (report press, start long-press timer)
 *                                    else --> IDLE (it was noise)
 *   PRESSED --edge IRQ (release)--> start 20 ms timer --> WAIT_RELEASE
 *   PRESSED --long-press timer (1 s)--> report LONG PRESS
 *   WAIT_RELEASE --timer: pin released?--> IDLE (report release)
 * Key points: never busy-wait in the ISR; disable the pin IRQ while debouncing so the
 * bounce doesn't flood the CPU with interrupts; act on the timer, not the edges.
 */
#include <stdio.h>
#include <stdint.h>

#define DEBOUNCE_MS 20
#define LONG_MS     1000

typedef enum { IDLE, WAIT_PRESS, PRESSED, WAIT_RELEASE } bstate_t;
static const char *names[] = {"IDLE", "WAIT_PRESS", "PRESSED", "WAIT_RELEASE"};

static bstate_t st = IDLE;
static int pin;                         /* 1 = pressed (after inversion) */
static int irq_enabled = 1;
static int32_t timer_left = -1;         /* one-shot timer, -1 = stopped */
static int32_t long_left = -1;
static uint32_t now;
static unsigned edge_irqs, ignored_edges;

static void goto_state(bstate_t s) { 
    if (s != st) printf("  t=%4u %s -> %s\n", (unsigned)now, names[st], names[s]); 
    st = s; 
}

static void pin_edge_isr(void)
{
    edge_irqs++;
    if (!irq_enabled) { 
        ignored_edges++; 
        return; 
    }       /* real HW: IRQ masked, never fires */
    irq_enabled = 0;
    timer_left = DEBOUNCE_MS;
    goto_state(st == PRESSED ? WAIT_RELEASE : WAIT_PRESS);
}

static void debounce_timer_cb(void)
{
    if (st == WAIT_PRESS) {
        if (pin) { printf("  t=%4u >>> PRESS\n", (unsigned)now); long_left = LONG_MS; goto_state(PRESSED); }
        else goto_state(IDLE);                           /* glitch */
    } else if (st == WAIT_RELEASE) {
        if (!pin) { printf("  t=%4u >>> RELEASE\n", (unsigned)now); long_left = -1; goto_state(IDLE); }
        else goto_state(PRESSED);                        /* bounce while held */
    }
    irq_enabled = 1;
}

static void long_timer_cb(void) { printf("  t=%4u >>> LONG PRESS\n", (unsigned)now); }

static void tick_1ms(void)
{
    if (timer_left > 0 && --timer_left == 0) { timer_left = -1; debounce_timer_cb(); }
    if (long_left > 0 && --long_left == 0) { long_left = -1; long_timer_cb(); }
}

static void set_pin(int v) { if (v != pin) { pin = v; pin_edge_isr(); } }

int main(void)
{
    /* press at 100 with 6 bounces, held 1.5 s, release at 1600 with bounces; noise spike at 2000 */
    for (now = 0; now < 2200; now++) {
        if (now >= 100 && now < 106) set_pin(now % 2 == 0);
        if (now == 106) set_pin(1);
        if (now >= 1600 && now < 1605) set_pin(now % 2);
        if (now == 1605) set_pin(0);
        if (now == 2000) set_pin(1);
        if (now == 2002) set_pin(0);                     /* 2 ms spike = noise */
        tick_1ms();
    }
    printf("edge interrupts: %u, ignored while debouncing: %u\n", edge_irqs, ignored_edges);
    return 0;
}
