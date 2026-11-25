/*
 * RTOS-06: O(1) priority scheduler - ready bitmap + per-priority FIFO lists
 * How FreeRTOS/uC-OS/ThreadX pick the next task in constant time:
 *   ready_bitmap bit p = 1 if any task of priority p is ready
 *   highest priority   = 31 - clz(bitmap)  (CLZ is one instruction on Cortex-M3+)
 *   tasks of equal priority are time-sliced round-robin (rotate the list each tick)
 * Simulated: no real context switch, we just show the scheduling decisions.
 */
#include <stdio.h>
#include <stdint.h>

#define MAX_PRIO  32
#define MAX_TASKS 8

typedef struct task { const char *name; int prio; struct task *next; } task_t;

static uint32_t ready_bitmap;
static task_t *ready_head[MAX_PRIO], *ready_tail[MAX_PRIO];

static void make_ready(task_t *t)
{
    t->next = NULL;
    if (ready_tail[t->prio]) ready_tail[t->prio]->next = t; else ready_head[t->prio] = t;
    ready_tail[t->prio] = t;
    ready_bitmap |= 1u << t->prio;
}

static task_t *pop_ready(int p)
{
    task_t *t = ready_head[p];
    ready_head[p] = t->next;
    if (!ready_head[p]) { ready_tail[p] = NULL; ready_bitmap &= ~(1u << p); }
    return t;
}

static int highest_ready(void) { return ready_bitmap ? 31 - __builtin_clz(ready_bitmap) : -1; }

/* one tick: run highest-priority task, then rotate it to the back (time slicing) */
static task_t *schedule_tick(void)
{
    int p = highest_ready();
    task_t *t;
    if (p < 0) return NULL;          /* would run the idle task */
    t = pop_ready(p);
    make_ready(t);
    return t;
}

int main(void)
{
    task_t idle_like = {"logger", 1, NULL}, a = {"motorA", 5, NULL}, b = {"motorB", 5, NULL}, h = {"estop", 9, NULL};
    int tick;

    make_ready(&idle_like); make_ready(&a); make_ready(&b);
    printf("bitmap=0x%08X highest=%d\n", (unsigned)ready_bitmap, highest_ready());
    for (tick = 0; tick < 10; tick++) {
        task_t *t;
        if (tick == 4) { make_ready(&h); printf("   -- estop becomes ready (ISR) --\n"); }
        if (tick == 6) { pop_ready(9);   printf("   -- estop blocks again --\n"); }
        t = schedule_tick();
        printf("tick %d: run %-7s (prio %d)\n", tick, t->name, t->prio);
    }
    printf("note: 'logger' (prio 1) never runs while prio 5 tasks are always ready -> starvation.\n");
    return 0;
}
