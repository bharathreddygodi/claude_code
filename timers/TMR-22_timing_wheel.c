/*
 * TMR-22: Hashed timing wheel - O(1) insert, O(1) cancel, cheap expiry
 * "1M connections, each with an idle timeout that resets on every packet."
 * A sorted list costs O(n) per insert; a heap costs O(log n). The wheel:
 *   - array of WHEEL slots, each a doubly linked list of timers
 *   - timer with deadline d goes into slot (d % WHEEL), with rounds = how many full
 *     turns of the wheel to wait (for deadlines further away than WHEEL ticks)
 *   - each tick: advance the cursor one slot, walk only that slot's list: rounds == 0
 *     -> fire, else rounds--
 *   - cancel = unlink from its slot, O(1). "touch" a connection = cancel + insert, O(1).
 * Per-tick cost is the size of one slot (~ n / WHEEL on average), not n.
 * Hierarchical wheel (Linux, Kafka): several wheels with coarser resolution
 * (ms / s / min, like a clock's hands); far-away timers sit in a coarse wheel and
 * cascade down, so no rounds counter and no re-walking long-lived timers.
 * Test: 10,000 random timers, 2,000 cancelled, 500 touched; every survivor must fire
 * exactly at its deadline.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define WHEEL 256u                        /* power of 2 -> slot = deadline & MASK */
#define MASK  (WHEEL - 1u)
#define NTIMERS 10000

typedef struct wtimer {
    struct wtimer *prev, *next;
    uint32_t deadline, rounds;
    bool linked;
    int fired_at;                         /* test bookkeeping */
} wtimer_t;

static wtimer_t *slots[WHEEL];
static uint32_t now;
static unsigned long work;                /* nodes visited, to show per-tick cost */

static void wheel_unlink(wtimer_t *t)
{
    uint32_t s = t->deadline & MASK;
    if (t->prev) t->prev->next = t->next; else slots[s] = t->next;
    if (t->next) t->next->prev = t->prev;
    t->prev = t->next = NULL;
    t->linked = false;
}

void wheel_start(wtimer_t *t, uint32_t delay)      /* delay >= 1 */
{
    uint32_t s;
    if (t->linked) wheel_unlink(t);
    t->deadline = now + delay;
    t->rounds = (delay - 1) / WHEEL;                /* full turns before it is due */
    s = t->deadline & MASK;
    t->prev = NULL;
    t->next = slots[s];
    if (slots[s]) slots[s]->prev = t;
    slots[s] = t;
    t->linked = true;
}

void wheel_cancel(wtimer_t *t) { if (t->linked) wheel_unlink(t); }

static void on_expire(wtimer_t *t) { t->fired_at = (int)now; }

void wheel_tick(void)
{
    wtimer_t *t, *next;
    now++;
    for (t = slots[now & MASK]; t; t = next) {
        next = t->next;                             /* t may be unlinked below */
        work++;
        if (t->rounds == 0) { wheel_unlink(t); on_expire(t); }
        else t->rounds--;
    }
}

static uint32_t rng = 12345;
static uint32_t rnd(uint32_t n) { rng = rng * 1103515245u + 12345u; return (rng >> 8) % n; }

static wtimer_t timers[NTIMERS];
static bool cancelled[NTIMERS];

int main(void)
{
    int i, ok = 0, bad = 0, ticks = 0, ncancel = 0;
    uint32_t max_deadline = 0;

    for (i = 0; i < NTIMERS; i++) {
        timers[i].fired_at = -1;
        wheel_start(&timers[i], 1 + rnd(3000));     /* up to ~12 turns of the wheel */
    }
    for (i = 0; i < 2000; i++) { int k = (int)rnd(NTIMERS); wheel_cancel(&timers[k]); cancelled[k] = true; }

    for (; ticks < 100; ticks++) wheel_tick();      /* 100 ms pass ... */
    for (i = 0; i < 500; i++) {                     /* ... then 500 connections get a packet */
        int k = (int)rnd(NTIMERS);
        if (!cancelled[k] && timers[k].linked) wheel_start(&timers[k], 2000);   /* reset idle timeout */
    }
    for (i = 0; i < NTIMERS; i++) if (timers[i].deadline > max_deadline) max_deadline = timers[i].deadline;
    while (now < max_deadline + 1) { wheel_tick(); ticks++; }

    for (i = 0; i < NTIMERS; i++) {
        if (cancelled[i]) { ncancel++; if (timers[i].fired_at != -1) bad++; continue; }   /* must never fire */
        if (timers[i].fired_at == (int)timers[i].deadline) ok++; else bad++;
    }
    printf("timers: %d, cancelled: %d (never fired), up to 500 touched\n", NTIMERS, ncancel);
    printf("fired exactly on deadline: %d, wrong: %d -> %s\n", ok, bad,
           (bad == 0 && ok + ncancel == NTIMERS) ? "PASS" : "FAIL");
    printf("ticks run: %d, nodes visited: %lu (%.1f per tick; a full scan would be ~%d per tick)\n",
           ticks, work, (double)work / ticks, NTIMERS);
    return 0;
}
