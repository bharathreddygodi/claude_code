/*
 * PTR-08: volatile and memory-mapped register access
 * On a real MCU:   #define GPIOA_ODR (*(volatile uint32_t *)0x40020014u)
 * Here we simulate the peripheral with a struct in RAM so it runs on a PC.
 *
 * volatile = every read/write really happens, in order (compiler can't cache it in
 * a register or delete "useless" loops/writes). Needed for: HW registers, variables
 * shared with ISRs, memory changed by DMA. It is NOT atomic and NOT a memory barrier.
 *
 * Interview Qs: Can a variable be both const and volatile? Yes - read-only status reg.
 *               Can a pointer be volatile? Yes - int *volatile p (pointer itself changes in ISR).
 *               What's wrong with:  int square(volatile int *p) { return *p * *p; }
 *               -> *p may change between the two reads; read once into a local.
 */
#include <stdio.h>
#include <stdint.h>

typedef struct {                    /* register map overlay, like CMSIS headers */
    volatile uint32_t MODER;        /* 0x00 */
    volatile uint32_t OTYPER;       /* 0x04 */
    volatile uint32_t OSPEEDR;      /* 0x08 */
    volatile uint32_t PUPDR;        /* 0x0C */
    volatile const uint32_t IDR;    /* 0x10 read-only */
    volatile uint32_t ODR;          /* 0x14 */
} gpio_t;

static gpio_t fake_gpioa;           /* real HW: #define GPIOA ((gpio_t *)0x40020000u) */
#define GPIOA (&fake_gpioa)

static volatile int data_ready;     /* set by an ISR, polled in main */
void uart_isr(void) { data_ready = 1; }

int square_safe(volatile int *p) { int v = *p; return v * v; }

int main(void)
{
    volatile int sample = 6;

    GPIOA->MODER = (GPIOA->MODER & ~(3u << 10)) | (1u << 10);  /* pin 5 = output */
    GPIOA->ODR |= (1u << 5);                                   /* LED on  */
    printf("MODER=0x%08X ODR=0x%08X\n", (unsigned)GPIOA->MODER, (unsigned)GPIOA->ODR);
    GPIOA->ODR &= ~(1u << 5);                                  /* LED off */
    printf("ODR after clear=0x%08X\n", (unsigned)GPIOA->ODR);

    printf("offset of ODR = 0x%02X (must match datasheet)\n",
           (unsigned)((uintptr_t)&GPIOA->ODR - (uintptr_t)GPIOA));

    uart_isr();                          /* pretend the interrupt fired */
    while (!data_ready) { }              /* without volatile: may be an infinite loop at -O2 */
    printf("data_ready seen, square_safe=%d\n", square_safe(&sample));
    return 0;
}
