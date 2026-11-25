/*
 * MEM-08: Memory layout of a C program + detect stack growth direction
 *   .text   : code, string literals (read-only, often .rodata)
 *   .data   : initialised globals/statics
 *   .bss    : zero/uninitialised globals/statics (zeroed at startup, no space in image)
 *   heap    : malloc, grows upward (usually)
 *   stack   : locals, return addresses, grows downward on most CPUs (x86, ARM)
 * Stack direction: compare the address of a local in a caller vs a callee.
 * (Strictly, comparing pointers to different objects is unspecified; casting to
 *  uintptr_t is the accepted interview answer.) noinline stops the compiler
 * from merging frames.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int g_init = 42;            /* .data */
int g_uninit;               /* .bss  */
const int g_const = 7;      /* .rodata */

#ifdef __GNUC__
__attribute__((noinline))
#endif
static int stack_grows_down(uintptr_t caller_local)
{
    volatile int callee_local = 0;
    return (uintptr_t)&callee_local < caller_local;
}

int main(void)
{
    volatile int local = 0;
    static int s_local = 1;   /* .data, even though declared in a function */
    int *heap = malloc(sizeof *heap);
    const char *lit = "literal";

    printf("text   (main)      : %p\n", (void *)(uintptr_t)main);
    printf("rodata (literal)   : %p\n", (const void *)lit);
    printf("rodata (g_const)   : %p\n", (const void *)&g_const);
    printf("data   (g_init)    : %p\n", (void *)&g_init);
    printf("data   (static loc): %p\n", (void *)&s_local);
    printf("bss    (g_uninit)  : %p\n", (void *)&g_uninit);
    printf("heap   (malloc)    : %p\n", (void *)heap);
    printf("stack  (local)     : %p\n", (void *)&local);
    printf("Stack grows %s\n", stack_grows_down((uintptr_t)&local) ? "DOWN (high -> low)" : "UP (low -> high)");
    free(heap);
    return 0;
}
