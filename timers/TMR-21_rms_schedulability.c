/*
 * TMR-21: Schedulability check for periods + WCETs under Rate Monotonic Scheduling
 * (RTOS-10 has the formulas; this version also SIMULATES the schedule to prove it.)
 *   1) sort by period (shorter period = higher priority)
 *   2) U = sum(C/T). U > 1 -> impossible on one CPU. U <= n(2^(1/n)-1) -> guaranteed.
 *   3) otherwise exact response-time analysis: R = C + sum_{higher j} ceil(R/Tj) * Cj
 *   4) cross-check: simulate a preemptive RM scheduler over the hyperperiod (LCM of periods)
 *      and record each task's worst observed response time and any deadline misses.
 * Integer math for RTA (no floating point needed on the target).
 */
#include <stdio.h>
#include <math.h>

typedef struct { const char *name; int C, T; } task_t;

static int gcd(int a, int b) { while (b) { int t = a % b; a = b; b = t; } return a; }

static void sort_rm(task_t *t, int n)
{
    int i, j;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && t[j].T < t[j - 1].T; j--) { task_t x = t[j]; t[j] = t[j - 1]; t[j - 1] = x; }
}

static void check(const char *title, task_t *t, int n)
{
    double U = 0, bound = n * (pow(2.0, 1.0 / n) - 1.0);
    int i, j, hyper = 1, rta_ok = 1, sim_ok = 1, tick;
    int R[8], remaining[8] = {0}, release[8] = {0}, worst[8] = {0}, misses[8] = {0};

    sort_rm(t, n);
    for (i = 0; i < n; i++) { U += (double)t[i].C / t[i].T; hyper = hyper / gcd(hyper, t[i].T) * t[i].T; }
    printf("== %s ==  U=%.3f  bound=%.3f  hyperperiod=%d\n", title, U, bound, hyper);

    for (i = 0; i < n; i++) {                     /* response-time analysis */
        int r = t[i].C, prev = -1;
        while (r != prev && r <= t[i].T) {
            prev = r;
            r = t[i].C;
            for (j = 0; j < i; j++) r += ((prev + t[j].T - 1) / t[j].T) * t[j].C;   /* integer ceil */
        }
        R[i] = r;
        if (r > t[i].T) rta_ok = 0;
    }

    for (tick = 0; tick < hyper; tick++) {         /* preemptive RM simulation, 1 unit per tick */
        for (i = 0; i < n; i++)
            if (tick % t[i].T == 0) {
                if (remaining[i] > 0) { misses[i]++; sim_ok = 0; }   /* previous job unfinished */
                remaining[i] = t[i].C;
                release[i] = tick;
            }
        for (i = 0; i < n && remaining[i] == 0; i++) { }           /* highest priority ready */
        if (i < n && --remaining[i] == 0 && tick + 1 - release[i] > worst[i]) worst[i] = tick + 1 - release[i];
    }

    for (i = 0; i < n; i++)
        printf("  %-3s C=%d T=%-3d RTA R=%-3d simulated worst=%-3d %s\n", t[i].name, t[i].C, t[i].T, R[i], worst[i],
               (R[i] <= t[i].T && !misses[i]) ? "ok" : "MISSES DEADLINE");
    printf("  verdict: %s (analysis %s simulation)\n\n", rta_ok ? "SCHEDULABLE" : "NOT schedulable",
           rta_ok == sim_ok ? "agrees with" : "DISAGREES with");
}

int main(void)
{
    task_t a[] = {{"T1", 1, 4}, {"T2", 2, 6}, {"T3", 1, 12}};
    task_t b[] = {{"T3", 1, 7}, {"T1", 1, 3}, {"T2", 2, 5}};    /* unsorted input on purpose */
    task_t c[] = {{"T1", 2, 4}, {"T2", 3, 7}, {"T3", 1, 9}};
    check("set A (under the bound)", a, 3);
    check("set B (above the bound, still OK)", b, 3);
    check("set C (overloaded)", c, 3);
    return 0;
}
