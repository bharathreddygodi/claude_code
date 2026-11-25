/*
 * INT-03: Interrupt priorities, nesting (preemption) and tail-chaining - simulation
 * Cortex-M NVIC: LOWER number = HIGHER priority. A pending IRQ preempts the running one
 * only if its priority is strictly higher. Equal/lower waits; when the current ISR
 * ends, the highest pending runs next WITHOUT restoring the main context (tail-chain).
 * Each ISR here takes several "cycles"; events arrive at given cycles.
 */
#include <stdio.h>

typedef struct { const char *name; int prio; int duration; } irq_t;

static irq_t irqs[] = {
    {"SysTick", 1, 2},
    {"UART",    2, 3},
    {"ADC",     3, 4},
};
#define N 3

typedef struct { int cycle, irq; } event_t;

int main(void)
{
    event_t ev[] = {{1, 2}, {3, 1}, {4, 0}, {5, 2}};   /* ADC@1, UART@3, SysTick@4, ADC@5 */
    int nev = 4, e = 0;
    int pending[N] = {0}, remaining[N] = {0};
    int stack[N], sp = 0;                              /* active ISR stack (nesting) */
    int cycle, i;

    for (cycle = 0; cycle < 20; cycle++) {
        while (e < nev && ev[e].cycle == cycle) {
            printf("cycle %2d: %s pending\n", cycle, irqs[ev[e].irq].name);
            pending[ev[e].irq] = 1;
            e++;
        }
        /* highest-priority pending */
        int best = -1;
        for (i = 0; i < N; i++)
            if (pending[i] && (best < 0 || irqs[i].prio < irqs[best].prio)) best = i;
        int cur = sp ? stack[sp - 1] : -1;
        if (best >= 0 && (cur < 0 || irqs[best].prio < irqs[cur].prio)) {
            if (cur >= 0) printf("cycle %2d:   %s PREEMPTS %s (nesting depth %d)\n", cycle, irqs[best].name, irqs[cur].name, sp + 1);
            else          printf("cycle %2d:   enter %s\n", cycle, irqs[best].name);
            pending[best] = 0;
            remaining[best] = irqs[best].duration;
            stack[sp++] = best;
            cur = best;
        }
        if (cur < 0) { if (e >= nev) break; continue; }   /* main code runs */

        if (--remaining[cur] == 0) {
            sp--;
            int next = -1;
            for (i = 0; i < N; i++)
                if (pending[i] && (next < 0 || irqs[i].prio < irqs[next].prio)) next = i;
            int under = sp ? stack[sp - 1] : -1;
            if (next >= 0 && (under < 0 || irqs[next].prio < irqs[under].prio))
                printf("cycle %2d:   exit %s -> TAIL-CHAIN to %s\n", cycle, irqs[cur].name, irqs[next].name);
            else
                printf("cycle %2d:   exit %s -> return to %s\n", cycle, irqs[cur].name, under < 0 ? "main" : irqs[under].name);
        }
    }
    printf("\nKey: the 2nd ADC (same priority as the running ADC) could not preempt it - it waited.\n");
    return 0;
}
