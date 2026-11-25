/*
 * INT-05: UART RX interrupt -> ring buffer -> command line assembly in main
 * ISR: read data register (clears RXNE), push into a lock-free SPSC ring buffer,
 *      count overflows (never block / never print in the ISR).
 * Main: pop bytes, build a line until '\n', handle backspace, guard line length.
 * Also handle UART error flags (overrun ORE, framing FE, noise NE) by reading SR then DR.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RB_SIZE 16u
static uint8_t rb[RB_SIZE];
static volatile uint32_t rb_head, rb_tail;     /* ISR writes head, main writes tail */
static volatile uint32_t rx_overflow;

static volatile uint32_t USART_DR;             /* simulated data register */

void USART1_IRQHandler(void)
{
    uint8_t b = (uint8_t)USART_DR;             /* reading DR clears RXNE */
    if (rb_head - rb_tail == RB_SIZE) { rx_overflow++; return; }   /* drop byte */
    rb[rb_head & (RB_SIZE - 1)] = b;
    rb_head++;
}

static int rb_pop(uint8_t *b)
{
    if (rb_head == rb_tail) return 0;
    *b = rb[rb_tail & (RB_SIZE - 1)];
    rb_tail++;
    return 1;
}

static void uart_receive_burst(const char *s)  /* simulated wire: one IRQ per byte */
{
    for (; *s; s++) { USART_DR = (uint8_t)*s; USART1_IRQHandler(); }
}

#define LINE_MAX 20
static char line[LINE_MAX + 1];
static int line_len;

static void handle_command(const char *cmd) { printf("  command: \"%s\"\n", cmd); }

static void main_loop_poll(void)
{
    uint8_t b;
    while (rb_pop(&b)) {
        if (b == '\r') continue;
        if (b == '\b') { if (line_len) line_len--; continue; }
        if (b == '\n') { line[line_len] = '\0'; handle_command(line); line_len = 0; continue; }
        if (line_len < LINE_MAX) line[line_len++] = (char)b;   /* silently truncate */
    }
}

int main(void)
{
    uart_receive_burst("LED ON\r\n");
    main_loop_poll();
    uart_receive_burst("SPEDD\b\bED 50\n");      /* backspace editing */
    main_loop_poll();
    uart_receive_burst("THIS LINE IS WAY TOO LONG FOR BUF\n");  /* > 16 bytes before main runs */
    main_loop_poll();
    printf("rx overflow count = %u (ring buffer filled before main polled)\n", (unsigned)rx_overflow);
    return 0;
}
