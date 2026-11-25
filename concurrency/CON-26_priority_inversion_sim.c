/*
 * CON-26: Priority inversion and priority inheritance (Mars Pathfinder, 1997)
 * Tasks: L (low) holds a mutex, H (high) needs it, M (medium) doesn't.
 *   Without inheritance: M preempts L, so H waits for M too -> unbounded inversion.
 *   With inheritance   : L is boosted to H's priority while holding the mutex,
 *                        M cannot preempt it, H gets the mutex as soon as L is done.
 * Other fix: priority ceiling (mutex gets the priority of its highest user).
 * This is a tick-by-tick simulation of a preemptive priority scheduler.
 */
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    int base_prio, prio;
    int arrive;       /* tick it becomes ready */
    int work;         /* ticks of work remaining */
    int cs_start;     /* work remaining when it wants the mutex (-1 = never) */
    int cs_len;       /* ticks inside the critical section */
    int in_cs, done, finish;
} task_t;

static void run(int inherit)
{
    task_t t[3] = {
        {"L", 1, 1, 0, 6, 6, 4, 0, 0, 0},   /* takes mutex immediately, holds for 4 ticks */
        {"H", 3, 3, 2, 3, 3, 1, 0, 0, 0},   /* arrives at 2, needs mutex */
        {"M", 2, 2, 3, 5, -1, 0, 0, 0, 0},  /* arrives at 3, CPU hog */
    };
    int owner = -1, tick, i;
    char timeline[64] = "";

    for (tick = 0; tick < 40; tick++) {
        int run_i = -1;
        for (i = 0; i < 3; i++) {         /* pick highest-priority ready, unblocked task */
            task_t *k = &t[i];
            int wants_mutex = !k->in_cs && k->work == k->cs_start;
            if (k->done || tick < k->arrive) continue;
            if (wants_mutex && owner != -1 && owner != i) {
                if (inherit && t[owner].prio < k->prio) t[owner].prio = k->prio;  /* boost owner */
                continue;                 /* blocked */
            }
            if (run_i < 0 || k->prio > t[run_i].prio) run_i = i;
        }
        if (run_i < 0) { strcat(timeline, "."); if (t[0].done && t[1].done && t[2].done) break; continue; }

        task_t *k = &t[run_i];
        if (!k->in_cs && k->work == k->cs_start) { owner = run_i; k->in_cs = k->cs_len; }
        strncat(timeline, k->name, 1);
        k->work--;
        if (k->in_cs && --k->in_cs == 0) { owner = -1; k->prio = k->base_prio; }  /* release, unboost */
        if (k->work == 0) { k->done = 1; k->finish = tick + 1; }
    }
    printf("%-22s timeline: %s\n", inherit ? "WITH inheritance" : "WITHOUT inheritance", timeline);
    printf("%-22s H finished at tick %d (arrived at 2)\n", "", t[1].finish);
}

int main(void)
{
    run(0);
    run(1);
    printf("-> without inheritance M runs before H even though H has higher priority.\n");
    printf("   FreeRTOS mutexes (xSemaphoreCreateMutex) have inheritance; binary semaphores do NOT.\n");
    return 0;
}
