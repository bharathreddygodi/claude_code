/*
 * DRV-02: UART driver - baud rate calculation, polled TX with timeout, interrupt TX queue
 *  - BRR = f_clk / baud  (oversampling by 16: mantissa/fraction); error % should be < ~2%
 *  - polled TX: wait for TXE (transmit data register empty) then write DR
 *    ALWAYS use a timeout in polling loops - a dead peripheral must not hang the system
 *  - interrupt TX: put bytes in a TX ring buffer, enable TXE interrupt, ISR feeds DR,
 *    disables TXE interrupt when the buffer is empty (else it fires forever)
 * Frame: start bit + 8 data (LSB first) + optional parity + stop -> 10 bits per byte at 8N1.
 */
#include <stdio.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t SR;       /* bit7 TXE, bit5 RXNE */
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;      /* bit7 TXEIE (TXE interrupt enable) */
} uart_regs_t;
#define SR_TXE   (1u << 7)
#define CR1_TXEIE (1u << 7)

static uart_regs_t fake_uart = {SR_TXE, 0, 0, 0};
#define UART1 (&fake_uart)

static char wire[128];          /* what went out on the TX pin */
static int wire_len;
static void hw_shift_out(void)  /* simulate the shifter finishing a byte */
{
    wire[wire_len++] = (char)UART1->DR;
    UART1->SR |= SR_TXE;
}

uint32_t uart_brr(uint32_t fclk, uint32_t baud) { return (fclk + baud / 2) / baud; }  /* rounded */

int uart_putc_polled(char c, uint32_t timeout)
{
    while (!(UART1->SR & SR_TXE)) {
        if (timeout-- == 0) return -1;          /* error instead of hanging */
    }
    UART1->DR = (uint8_t)c;
    UART1->SR &= ~SR_TXE;
    hw_shift_out();
    return 0;
}

/* interrupt-driven TX */
#define TXQ 32u
static char txq[TXQ];
static volatile uint32_t tx_head, tx_tail;

void USART1_IRQHandler(void)
{
    if ((UART1->SR & SR_TXE) && (UART1->CR1 & CR1_TXEIE)) {
        if (tx_head != tx_tail) {
            UART1->DR = (uint8_t)txq[tx_tail++ & (TXQ - 1)];
            UART1->SR &= ~SR_TXE;
        } else {
            UART1->CR1 &= ~CR1_TXEIE;           /* nothing left: stop the interrupt */
        }
    }
}

int uart_write_async(const char *s)
{
    for (; *s; s++) {
        if (tx_head - tx_tail == TXQ) return -1;   /* full */
        txq[tx_head++ & (TXQ - 1)] = *s;
    }
    UART1->CR1 |= CR1_TXEIE;                       /* kick off transmission */
    return 0;
}

int main(void)
{
    uint32_t fclk = 16000000u, bauds[] = {9600, 115200, 921600};
    unsigned i;
    for (i = 0; i < 3; i++) {
        uint32_t brr = uart_brr(fclk, bauds[i]);
        double actual = (double)fclk / brr, err = (actual - bauds[i]) * 100.0 / bauds[i];
        printf("baud %6u: BRR=%4u actual=%9.1f error=%+.2f%% %s\n", (unsigned)bauds[i], (unsigned)brr,
               actual, err, (err < 2 && err > -2) ? "OK" : "TOO HIGH");
    }
    printf("115200 8N1 -> %u bytes/s max\n", 115200u / 10u);

    for (const char *p = "polled\n"; *p; p++) uart_putc_polled(*p, 1000);
    UART1->SR &= ~SR_TXE;
    printf("putc with stuck TXE -> %d (timeout, no hang)\n", uart_putc_polled('x', 1000));
    UART1->SR |= SR_TXE;

    uart_write_async("async!\n");
    while (UART1->CR1 & CR1_TXEIE) {               /* hardware keeps firing the ISR */
        USART1_IRQHandler();
        if (!(UART1->SR & SR_TXE)) hw_shift_out();
    }
    printf("wire: %.*s", wire_len, wire);
    return 0;
}
