/*
 * DRV-01: GPIO driver over a memory-mapped register struct (STM32-style)
 *  - register map as a struct of volatile uint32_t at the peripheral base address
 *  - init: clock enable, mode, pull-up/down, output type
 *  - write via BSRR (bit set/reset register): ATOMIC, no read-modify-write,
 *    safe against ISRs touching other pins. ODR |= is a RMW race.
 * The "hardware" is a RAM struct; on target: #define GPIOA ((gpio_regs_t *)0x40020000u)
 */
#include <stdio.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER;    /* 2 bits/pin: 00 in, 01 out, 10 alt, 11 analog */
    volatile uint32_t OTYPER;   /* 1 bit/pin : 0 push-pull, 1 open-drain */
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;    /* 2 bits/pin: 00 none, 01 pull-up, 10 pull-down */
    volatile uint32_t IDR;      /* input data (read-only) */
    volatile uint32_t ODR;      /* output data */
    volatile uint32_t BSRR;     /* write 1 to bit n: set pin n; bit n+16: reset pin n */
} gpio_regs_t;

static gpio_regs_t fake_gpioa;
#define GPIOA (&fake_gpioa)

typedef enum { GPIO_IN = 0, GPIO_OUT = 1, GPIO_ALT = 2, GPIO_ANALOG = 3 } gpio_mode_t;
typedef enum { PULL_NONE = 0, PULL_UP = 1, PULL_DOWN = 2 } gpio_pull_t;

/* simulate what the hardware does when BSRR is written */
static void hw_apply_bsrr(gpio_regs_t *g)
{
    uint32_t v = g->BSRR;
    g->ODR = (g->ODR | (v & 0xFFFFu)) & ~(v >> 16);
    g->BSRR = 0;                                 /* BSRR reads as 0 */
}

void gpio_init(gpio_regs_t *g, unsigned pin, gpio_mode_t mode, gpio_pull_t pull, int open_drain)
{
    /* RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   enable the clock first! (classic bug) */
    g->MODER  = (g->MODER & ~(3u << (2 * pin))) | ((uint32_t)mode << (2 * pin));
    g->PUPDR  = (g->PUPDR & ~(3u << (2 * pin))) | ((uint32_t)pull << (2 * pin));
    g->OTYPER = (g->OTYPER & ~(1u << pin)) | ((uint32_t)(open_drain != 0) << pin);
}

void gpio_write(gpio_regs_t *g, unsigned pin, int high)
{
    g->BSRR = high ? (1u << pin) : (1u << (pin + 16));   /* single atomic write */
    hw_apply_bsrr(g);
}

void gpio_toggle(gpio_regs_t *g, unsigned pin)
{
    uint32_t odr = g->ODR;                       /* set the ones that are 0, reset the ones that are 1 */
    g->BSRR = ((odr & (1u << pin)) << 16) | (~odr & (1u << pin));
    hw_apply_bsrr(g);
}

int gpio_read(const gpio_regs_t *g, unsigned pin) { return (int)((g->IDR >> pin) & 1u); }

int main(void)
{
    gpio_init(GPIOA, 5, GPIO_OUT, PULL_NONE, 0);     /* LED */
    gpio_init(GPIOA, 0, GPIO_IN, PULL_UP, 0);        /* button, active low */
    printf("MODER=0x%08X PUPDR=0x%08X\n", (unsigned)GPIOA->MODER, (unsigned)GPIOA->PUPDR);

    gpio_write(GPIOA, 5, 1);  printf("LED on  : ODR=0x%04X\n", (unsigned)GPIOA->ODR);
    gpio_toggle(GPIOA, 5);    printf("toggle  : ODR=0x%04X\n", (unsigned)GPIOA->ODR);
    gpio_toggle(GPIOA, 5);    printf("toggle  : ODR=0x%04X\n", (unsigned)GPIOA->ODR);

    GPIOA->IDR = 0x0000;                              /* simulate button pressed (pulled low) */
    printf("button  : %s\n", gpio_read(GPIOA, 0) ? "released" : "PRESSED (active low)");
    return 0;
}
