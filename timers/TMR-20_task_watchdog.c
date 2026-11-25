/*
 * TMR-20: Task watchdog - each task checks in, kick the HW watchdog only if all did
 * (INT-08 shows the check-in bitmask. This version gives every task its OWN deadline,
 *  because a 10 ms control loop and a 5 s logger can't share one window.)
 *   task_checkin(id)       -> last_seen[id] = now
 *   supervisor (every 100 ms, lowest-priority task or timer):
 *       for each task: if now - last_seen > max_interval -> that task is stuck
 *       all healthy  -> kick HW watchdog (IWDG->KR = 0xAAAA)
 *       any stuck    -> DON'T kick; log the task id to no-init RAM so you know why after reset
 * Never kick from an ISR: interrupts keep running when every task is deadlocked.
 * HW watchdog timeout must exceed the supervisor period + worst-case jitter.
 */
#include <stdio.h>
#include <stdint.h>

#define HW_WDT_TIMEOUT 300           /* ms */
#define SUPERVISOR_MS  100

typedef struct { const char *name; uint32_t max_interval, last_seen, period; int alive; } wtask_t;
static wtask_t tasks[] = {
    {"control", 50,   0, 10,   1},
    {"comms",   500,  0, 200,  1},
    {"logger",  6000, 0, 5000, 1},
};
#define NT (sizeof tasks / sizeof tasks[0])

static uint32_t now, hw_wdt_deadline = HW_WDT_TIMEOUT;
static uint32_t noinit_reset_reason = 0xFFFFFFFFu;      /* survives reset in real HW */

static void task_checkin(unsigned id) { tasks[id].last_seen = now; }
static void hw_wdt_kick(void) { hw_wdt_deadline = now + HW_WDT_TIMEOUT; }

static void supervisor(void)
{
    unsigned i, healthy = 1;
    for (i = 0; i < NT; i++) {
        if (now - tasks[i].last_seen > tasks[i].max_interval) {
            if (healthy) printf("  t=%5u supervisor: '%s' silent for %u ms (limit %u) -> no kick\n", (unsigned)now,
                                tasks[i].name, (unsigned)(now - tasks[i].last_seen), (unsigned)tasks[i].max_interval);
            noinit_reset_reason = i;
            healthy = 0;
        }
    }
    if (healthy) hw_wdt_kick();
}

int main(void)
{
    unsigned i;
    for (now = 0; now < 20000; now++) {
        if (now == 7000) { tasks[1].alive = 0; printf("  t=%5u comms task deadlocks\n", (unsigned)now); }
        for (i = 0; i < NT; i++)
            if (tasks[i].alive && now % tasks[i].period == 0) task_checkin(i);
        if (now % SUPERVISOR_MS == 0) supervisor();
        if ((int32_t)(now - hw_wdt_deadline) >= 0) {
            printf("  t=%5u *** HW WATCHDOG RESET ***\n", (unsigned)now);
            break;
        }
    }
    printf("after reboot: reset reason in no-init RAM = task %u (%s)\n", (unsigned)noinit_reset_reason,
           noinit_reset_reason < NT ? tasks[noinit_reset_reason].name : "none");
    printf("the logger's 5 s check-in never tripped the watchdog: it has its own 6 s limit\n");
    return 0;
}
