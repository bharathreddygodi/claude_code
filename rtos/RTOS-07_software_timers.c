/*
 * RTOS-07: Software timers with a DELTA list (one hardware tick drives many timers)
 * Timers are kept sorted; each node stores ticks RELATIVE to the previous node.
 * The tick ISR only decrements the FIRST node -> O(1) per tick; insert is O(n).
 * Supports one-shot and auto-reload (periodic) timers with callbacks.
 * Interview points: callbacks run in timer-task/ISR context - keep them short and
 * non-blocking (FreeRTOS runs them in the timer service task, xTimerCreate()).
 */
#include <stdio.h>
#include <stdint.h>

typedef struct timer {
    const char *name;
    uint32_t period;          /* 0 = one-shot */
    uint32_t delta;           /* ticks after the previous timer in the list */
    void (*cb)(struct timer *);
    struct timer *next;
} timer_t_;

static timer_t_ *active;
static uint32_t now;

static void timer_start(timer_t_ *t, uint32_t ticks)
{
    timer_t_ **pp = &active;
    while (*pp && (*pp)->delta <= ticks) {      /* walk, consuming deltas */
        ticks -= (*pp)->delta;
        pp = &(*pp)->next;
    }
    t->delta = ticks;
    t->next = *pp;
    if (*pp) (*pp)->delta -= ticks;             /* keep the next node relative to us */
    *pp = t;
}

static void timer_tick(void)                    /* called from the tick ISR */
{
    now++;
    if (!active) return;
    if (active->delta) active->delta--;
    while (active && active->delta == 0) {      /* several may expire on one tick */
        timer_t_ *t = active;
        active = t->next;
        t->cb(t);
        if (t->period) timer_start(t, t->period);   /* auto-reload */
    }
}

static void on_expire(timer_t_ *t) { printf("tick %3u: %s fired\n", (unsigned)now, t->name); }

int main(void)
{
    timer_t_ blink = {"blink(50, periodic)", 50, 0, on_expire, NULL};
    timer_t_ tout  = {"timeout(120, one-shot)", 0, 0, on_expire, NULL};
    timer_t_ hb    = {"heartbeat(100, periodic)", 100, 0, on_expire, NULL};
    int i;

    timer_start(&blink, 50);
    timer_start(&tout, 120);
    timer_start(&hb, 100);
    for (i = 0; i < 300; i++) timer_tick();
    return 0;
}
