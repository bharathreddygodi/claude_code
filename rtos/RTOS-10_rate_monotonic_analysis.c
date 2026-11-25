/*
 * RTOS-10: Schedulability - Rate Monotonic (RM) and EDF
 * RM: fixed priorities, SHORTER period = HIGHER priority. Optimal among fixed-priority.
 *   Utilisation U = sum(Ci / Ti)
 *   Liu & Layland bound: U <= n(2^(1/n) - 1)   (~0.693 as n -> inf) -> guaranteed OK
 *   Above the bound it MAY still work -> do exact Response Time Analysis:
 *     R = Ci + sum over higher-prio j of ceil(R / Tj) * Cj   (iterate to a fixed point)
 *     schedulable if R <= Di (deadline, here = period)
 * EDF: dynamic priorities, schedulable iff U <= 1.
 */
#include <stdio.h>
#include <math.h>

typedef struct { const char *name; double C, T; } task_t;   /* worst-case exec time, period */

static void analyse(const char *title, task_t *t, int n)   /* t sorted by period (RM order) */
{
    double U = 0, bound = n * (pow(2.0, 1.0 / n) - 1.0);
    int i, j, ok = 1;

    for (i = 0; i < n; i++) U += t[i].C / t[i].T;
    printf("== %s ==\n U = %.3f, RM bound(n=%d) = %.3f -> %s\n", title, U, n, bound,
           U <= bound ? "schedulable by bound" : (U <= 1.0 ? "bound inconclusive, run RTA" : "overloaded (U>1)"));

    for (i = 0; i < n; i++) {
        double R = t[i].C, prev = 0;
        while (R != prev && R <= t[i].T) {
            prev = R;
            R = t[i].C;
            for (j = 0; j < i; j++) R += ceil(prev / t[j].T) * t[j].C;
        }
        printf("  %-3s C=%-4g T=%-4g response=%-5g %s\n", t[i].name, t[i].C, t[i].T, R,
               R <= t[i].T ? "meets deadline" : "MISSES deadline");
        if (R > t[i].T) ok = 0;
    }
    printf(" RM result: %s | EDF: %s\n\n", ok ? "SCHEDULABLE" : "NOT schedulable",
           U <= 1.0 ? "schedulable" : "not schedulable");
}

int main(void)
{
    task_t a[] = {{"T1", 1, 4}, {"T2", 2, 6}, {"T3", 1, 10}};
    task_t b[] = {{"T1", 1, 3}, {"T2", 2, 5}, {"T3", 1, 7}};     /* above bound but OK */
    task_t c[] = {{"T1", 2, 4}, {"T2", 3, 7}, {"T3", 1, 9}};     /* above bound, misses */
    analyse("set A", a, 3);
    analyse("set B", b, 3);
    analyse("set C", c, 3);
    return 0;
}
