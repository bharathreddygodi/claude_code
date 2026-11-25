/*
 * TMR-23: timer_start() from an ISR via a command queue drained by the timer task
 * (FreeRTOS xTimerStartFromISR / xTimerStopFromISR work exactly like this.)
 * Problem: the timer list belongs to the timer task. If an ISR edits it while the task
 * is halfway through an insert, the list is corrupted. Masking interrupts around every
 * list operation works but makes IRQ latency depend on how many timers exist.
 * Solution: the ISR only pushes a small command {op, timer, ms} into a lock-free SPSC
 * queue (O(1), no locks). The timer task is the ONLY code that touches the list:
 *   loop: drain all commands -> apply them -> run expired timers -> sleep until next
 * If the queue is full the ISR gets an error back (like pdFAIL) and must handle it:
 * count it, retry on the next interrupt, or size the queue for the worst burst.
 * Note: the deadline is computed from the tick when the ISR ran (stamped in the
 * command), so queueing delay doesn't stretch the timeout.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct stimer { struct stimer *next; uint32_t deadline; bool linked; const char *name; } stimer_t;
typedef enum { CMD_START, CMD_STOP } op_t;
typedef struct { op_t op; stimer_t *t; uint32_t ms, stamp; } cmd_t;

#define QSIZE 4u                               /* power of 2 */
static cmd_t q[QSIZE];
static volatile uint32_t q_head, q_tail;       /* ISR writes head, task writes tail */
static volatile uint32_t g_ticks;
static unsigned q_full_errors;

int timer_start_from_isr(stimer_t *t, uint32_t ms)
{
    if (q_head - q_tail == QSIZE) { q_full_errors++; return -1; }
    q[q_head & (QSIZE - 1)] = (cmd_t){CMD_START, t, ms, g_ticks};
    q_head++;                                  /* publish after writing (barrier on multi-core) */
    return 0;
}

int timer_stop_from_isr(stimer_t *t)
{
    if (q_head - q_tail == QSIZE) { q_full_errors++; return -1; }
    q[q_head & (QSIZE - 1)] = (cmd_t){CMD_STOP, t, 0, g_ticks};
    q_head++;
    return 0;
}

/* ---- everything below runs only in the timer task ---- */
static stimer_t *list;                         /* sorted by deadline */

static void list_remove(stimer_t *t)
{
    stimer_t **pp = &list;
    while (*pp && *pp != t) pp = &(*pp)->next;
    if (*pp) *pp = t->next;
    t->linked = false;
}

static void list_insert(stimer_t *t)
{
    stimer_t **pp = &list;
    while (*pp && (int32_t)((*pp)->deadline - t->deadline) <= 0) pp = &(*pp)->next;
    t->next = *pp;
    *pp = t;
    t->linked = true;
}

static void timer_task_iteration(void)
{
    uint32_t now = g_ticks;
    while (q_tail != q_head) {                 /* 1) drain commands */
        cmd_t c = q[q_tail & (QSIZE - 1)];
        q_tail++;
        if (c.t->linked) list_remove(c.t);
        if (c.op == CMD_START) { c.t->deadline = c.stamp + c.ms; list_insert(c.t); }
        printf("  t=%3u task: applied %s %s (queued at t=%u)\n", (unsigned)now,
               c.op == CMD_START ? "START" : "STOP ", c.t->name, (unsigned)c.stamp);
    }
    while (list && (int32_t)(now - list->deadline) >= 0) {   /* 2) run expired */
        stimer_t *t = list;
        list_remove(t);
        printf("  t=%3u task: %s EXPIRED (deadline %u)\n", (unsigned)now, t->name, (unsigned)t->deadline);
    }
}

int main(void)
{
    stimer_t rx_timeout = {0, 0, 0, "rx_timeout"}, led = {0, 0, 0, "led_off"}, burst[6];
    int i;
    for (i = 0; i < 6; i++) burst[i] = (stimer_t){0, 0, 0, "burst"};

    for (g_ticks = 0; g_ticks < 120; g_ticks++) {
        if (g_ticks == 10) timer_start_from_isr(&rx_timeout, 50);   /* UART ISR: byte arrived */
        if (g_ticks == 30) timer_start_from_isr(&rx_timeout, 50);   /* next byte: restart */
        if (g_ticks == 31) timer_start_from_isr(&led, 20);
        if (g_ticks == 40) timer_stop_from_isr(&led);               /* cancelled from ISR */
        if (g_ticks == 90)                                          /* IRQ storm: 6 starts, queue holds 4 */
            for (i = 0; i < 6; i++)
                if (timer_start_from_isr(&burst[i], 5) < 0) printf("  t=%3u ISR : queue FULL, start %d failed\n", (unsigned)g_ticks, i);
        if (g_ticks % 5 == 0 || g_ticks == 31) timer_task_iteration();   /* task runs when it gets CPU */
    }
    printf("queue-full errors reported to the ISR: %u\n", q_full_errors);
    return 0;
}
