/*
 * TMR-26: Logger Rate Limiter (LeetCode 359)
 * shouldPrintMessage(timestamp, msg): true if msg was NOT printed in the last 10 s.
 * Classic answer: hash map msg -> next allowed time. O(1) per call.
 * Embedded version (shown): no malloc, so a fixed-size open-addressing hash table
 * with copied keys. The interviewer's follow-up is "memory grows forever" ->
 *   - entries whose window has passed are reusable, so a lookup that lands on a stale
 *     entry can recycle it
 *   - if the table is full of live entries, decide a policy: allow the message
 *     (fail open, may spam) or drop it (fail closed, may lose a real error) - say which
 * Real-world use: throttling repeated fault logs ("sensor timeout" every 1 ms would
 * flood a UART and the flash log).
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define WINDOW  10
#define SLOTS   8u                       /* power of 2, small on purpose */
#define KEYLEN  24

typedef struct { char key[KEYLEN]; int next_ok; bool used; } entry_t;
static entry_t table[SLOTS];

static uint32_t hash(const char *s)      /* FNV-1a */
{
    uint32_t h = 2166136261u;
    while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; }
    return h;
}

bool should_print(int ts, const char *msg)
{
    uint32_t i, start = hash(msg) & (SLOTS - 1);
    int reuse = -1;
    for (i = 0; i < SLOTS; i++) {        /* linear probing */
        entry_t *e = &table[(start + i) & (SLOTS - 1)];
        if (!e->used) { if (reuse < 0) reuse = (int)((start + i) & (SLOTS - 1)); break; }
        if (strncmp(e->key, msg, KEYLEN) == 0) {
            if (ts < e->next_ok) return false;
            e->next_ok = ts + WINDOW;
            return true;
        }
        if (ts >= e->next_ok && reuse < 0) reuse = (int)((start + i) & (SLOTS - 1));   /* stale, recyclable */
    }
    if (reuse < 0) return true;          /* table full of live entries: fail open (policy!) */
    strncpy(table[reuse].key, msg, KEYLEN - 1);
    table[reuse].key[KEYLEN - 1] = '\0';
    table[reuse].next_ok = ts + WINDOW;
    table[reuse].used = true;
    return true;
}

int main(void)
{
    struct { int ts; const char *msg; bool expect; } tc[] = {   /* LeetCode example */
        {1, "foo", true}, {2, "bar", true}, {3, "foo", false},
        {8, "bar", false}, {10, "foo", false}, {11, "foo", true},
    };
    unsigned i;
    int printed = 0, t;

    for (i = 0; i < sizeof tc / sizeof tc[0]; i++) {
        bool r = should_print(tc[i].ts, tc[i].msg);
        printf("t=%2d %-4s -> %-5s %s\n", tc[i].ts, tc[i].msg, r ? "true" : "false", r == tc[i].expect ? "PASS" : "FAIL");
    }

    for (t = 100; t < 1100; t++)          /* a fault firing every ms for 1 s */
        printed += should_print(t, "sensor timeout");
    printf("'sensor timeout' raised 1000 times in 1000 ticks, printed %d times\n", printed);
    return 0;
}
