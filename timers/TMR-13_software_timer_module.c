/*
 * TMR-13: Software timer module - timer_start / timer_cancel / tick  (Appendix G)
 *
 * v1 simple: fixed table, countdown decremented in the tick ISR.
 *    Weak: O(N) work every tick inside the ISR, callbacks run in ISR context,
 *    fixed number of timers, integer ids can go stale (ABA), start/cancel need IRQ masking.
 *
 * v2 improved: ISR only does ticks++ (O(1)); timers are caller-owned nodes in a list
 *    SORTED by absolute deadline; timer_run() is called from main loop / timer task,
 *    so callbacks run in task context and may block briefly or call any API.
 *    - unlink BEFORE calling the callback -> the callback can cancel/restart itself
 *      or any other timer safely
 *    - periodic re-arm with deadline += period (no drift; see TMR-07)
 *    - wraparound-safe compares (TMR-14)
 *    - late main loop: CATCH_UP (fire every missed period) or SKIP (fire once, realign)
 *    - timer_next_deadline() lets the idle loop sleep until the next expiry (TMR-18)
 * Follow-ups: 10k timers -> timing wheel (TMR-22); start from ISR -> command queue (TMR-23).
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef void (*timer_cb_t)(void *arg);

/* ============================== v1 ============================== */
#define MAX_TIMERS 4
typedef struct { volatile bool active; uint32_t remaining, period; timer_cb_t cb; void *arg; } v1_timer_t;
static v1_timer_t g_v1[MAX_TIMERS];

int v1_start(uint32_t ms, timer_cb_t cb, void *arg, bool periodic)   /* returns id or -1 */
{
    int i;
    for (i = 0; i < MAX_TIMERS; i++) {
        if (!g_v1[i].active) {
            /* __disable_irq(); */
            g_v1[i].remaining = ms ? ms : 1;
            g_v1[i].period = periodic ? ms : 0;
            g_v1[i].cb = cb;
            g_v1[i].arg = arg;
            g_v1[i].active = true;               /* set last: ISR sees a complete entry */
            /* __enable_irq(); */
            return i;
        }
    }
    return -1;                                   /* table full */
}

void v1_cancel(int id) { if (id >= 0 && id < MAX_TIMERS) g_v1[id].active = false; }

void v1_tick(void)                               /* 1 ms ISR: O(MAX_TIMERS) every tick */
{
    int i;
    for (i = 0; i < MAX_TIMERS; i++) {
        v1_timer_t *t = &g_v1[i];
        if (t->active && --t->remaining == 0) {
            if (t->period) t->remaining = t->period; else t->active = false;
            t->cb(t->arg);                       /* runs in ISR context! */
        }
    }
}

/* ============================== v2 ============================== */
typedef enum { CATCH_UP, SKIP } late_policy_t;

typedef struct sw_timer {
    struct sw_timer *prev, *next;
    uint32_t deadline, period;
    timer_cb_t cb;
    void *arg;
    bool armed, linked;
    const char *name;
} sw_timer_t;                                    /* caller-owned: no malloc, no limit */

static volatile uint32_t g_ticks;
static sw_timer_t *g_head;
static late_policy_t g_policy = CATCH_UP;

static inline bool time_before(uint32_t a, uint32_t b) { return (int32_t)(a - b) < 0; }

static void list_remove(sw_timer_t *t)
{
    if (t->prev) t->prev->next = t->next; else g_head = t->next;
    if (t->next) t->next->prev = t->prev;
    t->prev = t->next = NULL;
    t->linked = false;
}

static void list_insert(sw_timer_t *t)          /* O(n); FIFO among equal deadlines */
{
    sw_timer_t *cur = g_head, *prev = NULL;
    while (cur && !time_before(t->deadline, cur->deadline)) { prev = cur; cur = cur->next; }
    t->prev = prev;
    t->next = cur;
    if (prev) prev->next = t; else g_head = t;
    if (cur) cur->prev = t;
    t->linked = true;
}

void timer_isr_tick(void) { g_ticks++; }        /* O(1) ISR */

void timer_init(sw_timer_t *t, const char *name, timer_cb_t cb, void *arg)
{
    memset(t, 0, sizeof *t);
    t->name = name; t->cb = cb; t->arg = arg;
}

void timer_start(sw_timer_t *t, uint32_t ms, bool periodic)
{
    if (t->linked) list_remove(t);              /* restart semantics */
    t->deadline = g_ticks + ms;                 /* 1 tick = 1 ms here */
    t->period = periodic ? ms : 0;
    t->armed = true;
    list_insert(t);
}

void timer_cancel(sw_timer_t *t) { t->armed = false; if (t->linked) list_remove(t); }

void timer_run(void)                            /* main loop / timer task */
{
    uint32_t now = g_ticks;                     /* snapshot once */
    while (g_head && !time_before(now, g_head->deadline)) {
        sw_timer_t *t = g_head;
        list_remove(t);                         /* unlink BEFORE callback */
        if (!t->period) t->armed = false;
        t->cb(t->arg);                          /* may cancel/restart anything */
        if (t->period && t->armed && !t->linked) {
            t->deadline += t->period;           /* no drift */
            if (g_policy == SKIP && !time_before(now, t->deadline)) {
                uint32_t missed = (now - t->deadline) / t->period + 1;
                t->deadline += missed * t->period;   /* jump past now, fire once */
            }
            list_insert(t);
        }
    }
}

bool timer_next_deadline(uint32_t *out)
{
    if (!g_head) return false;
    *out = g_head->deadline;
    return true;
}

/* ============================== tests ============================== */
static char g_log[512];
static void log_event(const char *s)
{
    char buf[48];
    snprintf(buf, sizeof buf, "%s@%u ", s, (unsigned)g_ticks);
    strncat(g_log, buf, sizeof g_log - strlen(g_log) - 1);
}
static void reset(void) { g_head = NULL; g_ticks = 0; g_log[0] = 0; g_policy = CATCH_UP; }
static void run_ticks(int n) { while (n--) { timer_isr_tick(); timer_run(); } }
static void check(const char *test, const char *expect)
{
    printf("%-30s %s\n   got:    %s\n", test, strcmp(g_log, expect) ? "FAIL" : "PASS", g_log);
    if (strcmp(g_log, expect)) printf("   expect: %s\n", expect);
}

static sw_timer_t A, B, C;
static void cb_log(void *arg)          { log_event(((sw_timer_t *)arg)->name); }
static void cb_cancel_self(void *arg)  { sw_timer_t *t = arg; log_event(t->name); if (g_ticks >= 6) timer_cancel(t); }
static void cb_cancel_other(void *arg) { log_event(((sw_timer_t *)arg)->name); timer_cancel(&B); }
static int restarts;
static void cb_restart_self(void *arg) { sw_timer_t *t = arg; log_event(t->name); if (++restarts < 3) timer_start(t, 5, false); }
static void v1_cb(void *arg)           { printf("  v1 %s fired (inside the ISR)\n", (const char *)arg); }

int main(void)
{
    uint32_t nd = 0;
    int i;

    printf("=== v1: table + countdown in ISR ===\n");
    v1_start(3, v1_cb, "T3 one-shot", false);
    v1_start(4, v1_cb, "T4 periodic", true);
    for (i = 0; i < 8; i++) v1_tick();
    {
        int x = v1_start(1, v1_cb, "x", 0), y = v1_start(1, v1_cb, "y", 0);
        int z = v1_start(1, v1_cb, "z", 0), w = v1_start(1, v1_cb, "w", 0);
        printf("  4 more starts -> ids %d %d %d %d (-1 = table full, fixed size)\n", x, y, z, w);
    }

    printf("\n=== v2: sorted list, callbacks in task context ===\n");
    reset(); timer_init(&A, "A", cb_log, &A);
    timer_start(&A, 5, false); run_ticks(10);
    check("one-shot 5ms", "A@5 ");

    reset(); timer_init(&A, "P", cb_log, &A);
    timer_start(&A, 3, true); run_ticks(10); timer_cancel(&A); run_ticks(10);
    check("periodic 3ms, cancel at 10", "P@3 P@6 P@9 ");

    reset(); timer_init(&A, "A", cb_log, &A); timer_init(&B, "B", cb_log, &B); timer_init(&C, "C", cb_log, &C);
    timer_start(&A, 4, false); timer_start(&B, 4, false); timer_start(&C, 4, false); run_ticks(5);
    check("equal deadlines keep FIFO", "A@4 B@4 C@4 ");

    reset(); timer_init(&A, "S", cb_cancel_self, &A);
    timer_start(&A, 2, true); run_ticks(12);
    check("cancel self inside callback", "S@2 S@4 S@6 ");

    reset(); timer_init(&A, "A", cb_cancel_other, &A); timer_init(&B, "B", cb_log, &B);
    timer_start(&A, 3, false); timer_start(&B, 3, false); run_ticks(5);
    check("A cancels B due same tick", "A@3 ");

    reset(); restarts = 0; timer_init(&A, "R", cb_restart_self, &A);
    timer_start(&A, 5, false); run_ticks(20);
    check("one-shot restarts itself", "R@5 R@10 R@15 ");

    reset(); g_ticks = UINT32_MAX - 3; timer_init(&A, "W", cb_log, &A);
    timer_start(&A, 6, false); run_ticks(4); log_event("wrap"); run_ticks(3);
    check("wraparound near UINT32_MAX", "wrap@0 W@2 ");

    reset(); timer_init(&A, "L", cb_log, &A);
    timer_start(&A, 2, true); g_ticks = 7; timer_run();          /* main loop was late */
    check("late loop, CATCH_UP policy", "L@7 L@7 L@7 ");
    reset(); g_policy = SKIP; timer_init(&A, "L", cb_log, &A);
    timer_start(&A, 2, true); g_ticks = 7; timer_run(); run_ticks(1);
    check("late loop, SKIP policy", "L@7 L@8 ");

    reset(); timer_init(&A, "A", cb_log, &A); timer_init(&B, "B", cb_log, &B);
    timer_start(&B, 50, false); timer_start(&A, 20, false);
    {
        bool have = timer_next_deadline(&nd);
        printf("%-30s %s (next deadline = %u, idle can sleep %u ticks)\n", "timer_next_deadline()",
               (have && nd == 20) ? "PASS" : "FAIL", (unsigned)nd, (unsigned)(nd - g_ticks));
    }
    return 0;
}
