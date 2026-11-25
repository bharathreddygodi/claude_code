/*
 * RTOS-05: Cooperative (time-triggered) scheduler - "super loop with a task table"
 * Each task has a period; a 1 ms tick ISR increments a counter; the main loop runs
 * every task whose time has come. Tasks must run to completion quickly (no blocking).
 * Pros: simple, no races between tasks, tiny RAM. Cons: one slow task delays all,
 * no preemption -> worst-case latency = sum of task run times.
 * The tick ISR is simulated by incrementing g_ticks in the loop.
 */
#include <stdio.h>
#include <stdint.h>

typedef struct {
    const char *name;
    void (*fn)(void);
    uint32_t period_ms;
    uint32_t last_run;
} task_t;

static volatile uint32_t g_ticks;              /* incremented by SysTick_Handler */
void SysTick_Handler(void) { g_ticks++; }

static void led_task(void)    { printf("[%4u] led toggle\n", (unsigned)g_ticks); }
static void sensor_task(void) { printf("[%4u]   sensor read\n", (unsigned)g_ticks); }
static void comms_task(void)  { printf("[%4u]     comms send\n", (unsigned)g_ticks); }

static task_t tasks[] = {
    {"led",    led_task,    500, 0},
    {"sensor", sensor_task, 200, 0},
    {"comms",  comms_task,  1000, 0},
};
#define NTASKS (sizeof tasks / sizeof tasks[0])

static void scheduler_run_once(void)
{
    unsigned i;
    uint32_t now = g_ticks;                    /* read the volatile once */
    for (i = 0; i < NTASKS; i++) {
        if ((uint32_t)(now - tasks[i].last_run) >= tasks[i].period_ms) {   /* wrap-safe */
            tasks[i].last_run = now;
            tasks[i].fn();
        }
    }
}

int main(void)
{
    while (g_ticks < 2000) {                   /* real firmware: for (;;) */
        SysTick_Handler();                     /* simulated 1 ms tick */
        scheduler_run_once();
        /* __WFI();  sleep until next interrupt to save power */
    }
    printf("note: (now - last) >= period works even when the tick counter wraps.\n");
    return 0;
}
