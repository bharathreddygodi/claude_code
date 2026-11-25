/*
 * INT-07: Reentrancy - why some functions must never be called from an ISR
 * A function is reentrant if it can be interrupted mid-way and called again (from an
 * ISR or another task) without corrupting anything. Breaks reentrancy:
 *   - static/global state (static buffers, strtok, rand, asctime)
 *   - non-atomic access to shared HW or data
 *   - calling non-reentrant functions (printf, malloc - they use locks/globals)
 * Fix: use only locals/parameters, caller-provided buffers, or protect with a
 * critical section. (Thread-safe != reentrant: a mutex-protected fn can deadlock
 * if an ISR re-enters it.)
 * Simulation: a hook fires the "ISR" in the middle of the function.
 */
#include <stdio.h>
#include <string.h>

static void (*interrupt_hook)(void);

/* NOT reentrant: result lives in a static buffer */
static char *fmt_temp_bad(int t)
{
    static char buf[32];
    snprintf(buf, sizeof buf, "temp=%d", t);
    if (interrupt_hook) { void (*h)(void) = interrupt_hook; interrupt_hook = NULL; h(); }  /* ISR hits here */
    return buf;
}

/* Reentrant: caller provides the storage */
static char *fmt_temp_good(int t, char *buf, size_t n)
{
    snprintf(buf, n, "temp=%d", t);
    if (interrupt_hook) { void (*h)(void) = interrupt_hook; interrupt_hook = NULL; h(); }
    return buf;
}

static char isr_result[32];
static void isr_bad(void)  { strcpy(isr_result, fmt_temp_bad(99)); }
static void isr_good(void) { char b[32]; strcpy(isr_result, fmt_temp_good(99, b, sizeof b)); }

int main(void)
{
    char mine[32];
    char *r;

    interrupt_hook = isr_bad;
    r = fmt_temp_bad(25);
    printf("non-reentrant: main expected \"temp=25\", got \"%s\" -> %s\n", r, strcmp(r, "temp=25") ? "CORRUPTED" : "ok");

    interrupt_hook = isr_good;
    r = fmt_temp_good(25, mine, sizeof mine);
    printf("reentrant    : main expected \"temp=25\", got \"%s\" -> %s (isr got \"%s\")\n",
           r, strcmp(r, "temp=25") ? "CORRUPTED" : "PASS", isr_result);
    return 0;
}
