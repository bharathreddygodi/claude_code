/*
 * PTR-04: Function pointers
 *  - declare/typedef, pass as callback, return from a function
 *  - array of function pointers: command table / jump table
 *  - state machine driven by a table of handlers (very common in embedded)
 *  - ISR vector table is just an array of function pointers
 */
#include <stdio.h>
#include <string.h>

typedef int (*binop_t)(int, int);

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }

int apply(binop_t f, int a, int b) { return f(a, b); }  /* callback */

binop_t pick(char op)                                  /* returning a fn ptr */
{
    switch (op) { case '+': return add; case '-': return sub; case '*': return mul; }
    return NULL;
}

/* Command table */
typedef struct { const char *name; void (*handler)(void); } cmd_t;
static void cmd_led(void)   { printf("  -> LED toggled\n"); }
static void cmd_reset(void) { printf("  -> system reset\n"); }
static const cmd_t cmds[] = { {"led", cmd_led}, {"reset", cmd_reset} };

static void dispatch(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof cmds / sizeof cmds[0]; i++)
        if (strcmp(cmds[i].name, name) == 0) { cmds[i].handler(); return; }
    printf("  -> unknown command '%s'\n", name);
}

/* Table-driven state machine */
typedef enum { IDLE, RUNNING, ERROR_ST, NUM_STATES } state_t;
typedef state_t (*state_fn)(int event);
static state_t on_idle(int e)    { printf("  IDLE    evt=%d\n", e); return e == 1 ? RUNNING : IDLE; }
static state_t on_running(int e) { printf("  RUNNING evt=%d\n", e); return e == 9 ? ERROR_ST : (e == 0 ? IDLE : RUNNING); }
static state_t on_error(int e)   { printf("  ERROR   evt=%d\n", e); return IDLE; }
static const state_fn fsm[NUM_STATES] = { on_idle, on_running, on_error };

int main(void)
{
    int (*fp)(int, int) = add;               /* without typedef */
    binop_t ops[] = { add, sub, mul };       /* jump table */
    int events[] = {1, 5, 9, 0, 1, 0};
    state_t s = IDLE;
    unsigned i;

    printf("fp(2,3)=%d (*fp)(2,3)=%d\n", fp(2, 3), (*fp)(2, 3));
    printf("apply(mul,4,5)=%d pick('-')(9,4)=%d ops[0](1,1)=%d\n", apply(mul, 4, 5), pick('-')(9, 4), ops[0](1, 1));

    dispatch("led"); dispatch("reset"); dispatch("boom");

    printf("state machine:\n");
    for (i = 0; i < sizeof events / sizeof events[0]; i++)
        s = fsm[s](events[i]);
    return 0;
}
