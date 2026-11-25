/*
 * CON-21: ISR basics - flag + deferred processing (top half / bottom half)
 * ISR rules (say these in the interview):
 *   - keep it SHORT: read the HW, clear the interrupt flag, save data, signal, return
 *   - no blocking calls, no printf, no malloc, no floating point (unless FPU context saved)
 *   - shared variables are volatile; multi-byte shared data needs a critical section
 *   - clear the peripheral's pending flag or the ISR re-fires forever
 *   - no return value, no parameters (void ISR(void))
 * Heavy work goes to the main loop / a task ("bottom half", deferred procedure).
 * Hardware is simulated: hw_event() plays the role of the NVIC calling the ISR.
 */
#include <stdio.h>
#include <stdint.h>

/* --- simulated peripheral registers --- */
static volatile uint32_t ADC_DR;          /* data register */
static volatile uint32_t ADC_SR;          /* status: bit0 = EOC (end of conversion) */
#define ADC_SR_EOC (1u << 0)

/* --- shared between ISR and main --- */
static volatile uint8_t  sample_ready;
static volatile uint16_t latest_sample;
static volatile uint32_t overrun_count;

void ADC_IRQHandler(void)                 /* top half: microseconds */
{
    if (ADC_SR & ADC_SR_EOC) {
        if (sample_ready) overrun_count++;   /* main loop didn't keep up */
        latest_sample = (uint16_t)ADC_DR;    /* reading DR also clears EOC on many MCUs */
        ADC_SR &= ~ADC_SR_EOC;               /* clear the flag explicitly */
        sample_ready = 1;
    }
}

static void hw_event(uint16_t value)      /* simulate conversion finishing */
{
    ADC_DR = value;
    ADC_SR |= ADC_SR_EOC;
    ADC_IRQHandler();
}

static void process_sample(uint16_t s)    /* bottom half: slow work is fine here */
{
    uint32_t mv = (uint32_t)s * 3300u / 4095u;
    printf("  main: raw=%4u -> %4u mV%s\n", s, (unsigned)mv, mv > 3000 ? "  (ALARM)" : "");
}

int main(void)
{
    uint16_t inputs[] = {100, 2048, 4000, 3900, 512};
    unsigned i;

    for (i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
        hw_event(inputs[i]);
        if (i == 2) hw_event(4095);       /* two interrupts before main runs -> overrun */
        if (sample_ready) {               /* main loop polls the flag */
            uint16_t s = latest_sample;
            sample_ready = 0;
            process_sample(s);
        }
    }
    printf("overruns: %u (one sample was overwritten before main read it)\n", (unsigned)overrun_count);
    return 0;
}
