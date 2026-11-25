/*
 * INT-02: Interrupt vector table and dynamic handler registration
 * On Cortex-M the vector table is an array of function pointers at address 0
 * (or wherever VTOR points): [0] initial SP, [1] Reset_Handler, [2] NMI, ...,
 * [16+n] IRQn handlers. Unused entries point to Default_Handler (infinite loop).
 * Weak symbols let drivers override handlers:  void UART_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
 * Relocating the table to RAM (VTOR) allows registering handlers at runtime - shown here.
 */
#include <stdio.h>
#include <stdint.h>

typedef void (*isr_t)(void);

#define NUM_IRQ 8
enum { IRQ_TIMER = 0, IRQ_UART = 1, IRQ_GPIO = 2 };

static void Default_Handler(void) { printf("  Default_Handler: unexpected IRQ (real HW: while(1) / log + reset)\n"); }

static isr_t vector_table[NUM_IRQ];       /* RAM copy, like after setting SCB->VTOR */
static volatile uint32_t irq_enabled;     /* like NVIC->ISER */

void irq_init(void)
{
    int i;
    for (i = 0; i < NUM_IRQ; i++) vector_table[i] = Default_Handler;
}

int irq_register(int irq, isr_t h)
{
    if (irq < 0 || irq >= NUM_IRQ || !h) return -1;
    vector_table[irq] = h;
    return 0;
}

void irq_enable(int irq)  { irq_enabled |=  (1u << irq); }
void irq_disable(int irq) { irq_enabled &= ~(1u << irq); }

static void hw_raise(int irq)            /* what the NVIC does */
{
    printf("IRQ %d raised: ", irq);
    if (!(irq_enabled & (1u << irq))) { printf("masked, ignored (stays pending on real HW)\n"); return; }
    printf("\n");
    vector_table[irq]();
}

static volatile uint32_t ticks;
static void timer_isr(void) { ticks++; printf("  timer_isr: ticks=%u\n", (unsigned)ticks); }
static void uart_isr(void)  { printf("  uart_isr: byte received\n"); }

int main(void)
{
    irq_init();
    irq_register(IRQ_TIMER, timer_isr);
    irq_register(IRQ_UART, uart_isr);
    irq_enable(IRQ_TIMER);
    irq_enable(IRQ_GPIO);                /* enabled but no handler registered */

    hw_raise(IRQ_TIMER);
    hw_raise(IRQ_UART);                  /* not enabled yet */
    irq_enable(IRQ_UART);
    hw_raise(IRQ_UART);
    hw_raise(IRQ_GPIO);                  /* falls to Default_Handler */
    irq_disable(IRQ_TIMER);
    hw_raise(IRQ_TIMER);
    return 0;
}
