/*
 * TMR-17: Cooperative (time-triggered) scheduler with a task table
 *   task = { name, period, next_run, fn, priority, budget }
 *   Each loop: among tasks that are due, run the highest priority one; one task at a
 *   time, each runs to completion (no preemption).
 *   next_run += period          -> no drift (TMR-07)
 *   if we fall a whole period behind -> count a MISSED deadline and skip ahead
 *   if a task runs longer than its budget -> count an OVERRUN (it delays everyone)
 * Simulated: a task "takes" some ms by advancing the clock.
 */
#include <stdio.h>
#include <stdint.h>

static uint32_t g_ms;
#define time_due(now, t) ((int32_t)((now) - (t)) >= 0)

typedef struct {
    const char *name;
    uint32_t period, next_run, budget;
    int priority;                    /* higher = more important */
    void (*fn)(void);
    unsigned runs, missed, overruns;
} task_t;

static int comms_slow;
static void motor(void)  { g_ms += 1; }
static void sensor(void) { g_ms += 2; }
static void comms(void)  { g_ms += comms_slow ? 25 : 3; }   /* occasionally very slow */
static void logger(void) { g_ms += 4; }

static task_t tasks[] = {
    {"motor",  5,  0, 2, 3, motor,  0, 0, 0},
    {"sensor", 10, 0, 3, 2, sensor, 0, 0, 0},
    {"comms",  20, 0, 5, 1, comms,  0, 0, 0},
    {"logger", 50, 0, 6, 0, logger, 0, 0, 0},
};
#define NT (sizeof tasks / sizeof tasks[0])

static void scheduler_step(void)
{
    task_t *pick = NULL;
    unsigned i;
    uint32_t now = g_ms, start;

    for (i = 0; i < NT; i++)
        if (time_due(now, tasks[i].next_run) && (!pick || tasks[i].priority > pick->priority))
            pick = &tasks[i];
    if (!pick) { g_ms++; return; }                  /* idle: real code would __WFI() */

    if ((uint32_t)(now - pick->next_run) >= pick->period) {   /* a whole period late */
        uint32_t late = (now - pick->next_run) / pick->period;
        pick->missed += late;
        pick->next_run += late * pick->period;      /* skip the missed releases */
        printf("  t=%3u %-6s MISSED %u release(s)\n", (unsigned)now, pick->name, (unsigned)late);
    }
    start = g_ms;
    pick->fn();
    pick->runs++;
    if (g_ms - start > pick->budget) {
        pick->overruns++;
        printf("  t=%3u %-6s OVERRUN: took %u ms, budget %u ms\n", (unsigned)start, pick->name,
               (unsigned)(g_ms - start), (unsigned)pick->budget);
    }
    pick->next_run += pick->period;                 /* no drift */
}

int main(void)
{
    unsigned i;
    while (g_ms < 200) {
        comms_slow = (g_ms >= 60 && g_ms < 70);     /* one slow comms run around t=60 */
        scheduler_step();
    }
    printf("\n%-7s %6s %5s %6s %8s\n", "task", "period", "runs", "missed", "overruns");
    for (i = 0; i < NT; i++)
        printf("%-7s %6u %5u %6u %8u\n", tasks[i].name, (unsigned)tasks[i].period, tasks[i].runs,
               tasks[i].missed, tasks[i].overruns);
    printf("\nOne 25 ms comms run made motor (5 ms period) miss releases: no preemption.\n"
           "Fixes: split long work into steps (state machine), move it to an ISR/DMA, or use an RTOS.\n");
    return 0;
}
