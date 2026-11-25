/*
 * INT-09: Edge vs level triggering and lost interrupts
 * Edge-triggered: the pending bit is ONE bit. If 3 edges arrive while the ISR is busy
 *   (or masked), they collapse into one pending interrupt -> 2 events lost.
 *   Fix: ISR must check the HW for ALL work (read a counter/FIFO, loop until empty),
 *   never assume "one interrupt = one event".
 * Level-triggered: IRQ stays asserted while the condition holds -> must clear the
 *   source in the ISR or it re-enters forever (interrupt storm).
 * Simulation counts events vs ISR runs.
 */
#include <stdio.h>

static int pending;            /* single pending bit */
static int hw_fifo_count;      /* what the peripheral really has (e.g. RX FIFO level) */
static int handled_naive, handled_fifo;

static void edge(void) { pending = 1; hw_fifo_count++; }

static void isr_naive(void) { handled_naive++; hw_fifo_count--; }          /* one per IRQ */
static void isr_fifo(void)  { while (hw_fifo_count) { hw_fifo_count--; handled_fifo++; } }

int main(void)
{
    int round, isr_runs = 0, storm = 0, i;

    /* naive ISR: 3 edges arrive while masked/busy each round */
    for (round = 0; round < 4; round++) {
        edge(); edge(); edge();
        if (pending) { pending = 0; isr_naive(); isr_runs++; }
    }
    printf("naive ISR : 12 events, %d ISR runs, handled %d -> %d LOST (still in FIFO)\n",
           isr_runs, handled_naive, 12 - handled_naive);

    hw_fifo_count = 0; isr_runs = 0;
    for (round = 0; round < 4; round++) {
        edge(); edge(); edge();
        if (pending) { pending = 0; isr_fifo(); isr_runs++; }
    }
    printf("drain ISR : 12 events, %d ISR runs, handled %d -> none lost\n", isr_runs, handled_fifo);

    /* level-triggered: line stays high until the ISR clears the source */
    {
        int line_high = 1, clears_source = 0;
        for (i = 0; i < 1000 && line_high; i++) { storm++; if (clears_source) line_high = 0; }
        printf("level IRQ, source NOT cleared: ISR re-entered %d times (storm, main starves)\n", storm);
        storm = 0; line_high = 1; clears_source = 1;
        for (i = 0; i < 1000 && line_high; i++) { storm++; if (clears_source) line_high = 0; }
        printf("level IRQ, source cleared    : ISR ran %d time\n", storm);
    }
    return 0;
}
