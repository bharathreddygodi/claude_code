/*
 * CON-12: DMA with circular double buffering (ping-pong) - e.g. ADC or audio stream
 * DMA fills a buffer of 2*N samples in circular mode and raises:
 *   Half-Transfer (HT) interrupt -> first half is full, CPU processes it while DMA fills 2nd
 *   Transfer-Complete (TC)       -> second half full, CPU processes it while DMA wraps to 1st
 * CPU never touches the half the DMA is writing. If processing takes longer than one
 * half-buffer time -> overrun (data corrupted) -> detect it.
 * Cache gotcha on Cortex-M7/A: invalidate D-cache for the DMA'd region before reading
 * (or put buffers in non-cacheable memory). Buffers often need alignment too.
 */
#include <stdio.h>
#include <stdint.h>

#define HALF 4
static uint16_t dma_buf[2 * HALF];
static volatile int half_ready = -1;      /* 0 = first half, 1 = second half */
static volatile int overrun;
static int dma_pos;
static uint16_t next_sample = 100;

static void DMA_IRQHandler(int tc)
{
    if (half_ready != -1) overrun++;      /* previous half not processed yet */
    half_ready = tc;                       /* HT -> 0, TC -> 1 */
}

static void dma_hw_step(void)              /* DMA moves one sample from ADC to memory */
{
    dma_buf[dma_pos++] = next_sample++;
    if (dma_pos == HALF) DMA_IRQHandler(0);
    if (dma_pos == 2 * HALF) { dma_pos = 0; DMA_IRQHandler(1); }   /* circular */
}

static void process(int half)
{
    const uint16_t *p = &dma_buf[half * HALF];
    uint32_t sum = 0;
    int i;
    for (i = 0; i < HALF; i++) sum += p[i];
    printf("  process %s half: %u %u %u %u avg=%u\n", half ? "2nd" : "1st", p[0], p[1], p[2], p[3], (unsigned)(sum / HALF));
}

int main(void)
{
    int step;
    printf("CPU keeps up:\n");
    for (step = 0; step < 16; step++) {
        dma_hw_step();
        if (half_ready != -1) { int h = half_ready; half_ready = -1; process(h); }
    }
    printf("overruns=%d\n\nCPU too slow (only checks every 10 samples):\n", overrun);
    for (step = 0; step < 24; step++) {
        dma_hw_step();
        if (step % 10 == 9 && half_ready != -1) { int h = half_ready; half_ready = -1; process(h); }
    }
    printf("overruns=%d -> buffers were overwritten before processing\n", overrun);
    return 0;
}
