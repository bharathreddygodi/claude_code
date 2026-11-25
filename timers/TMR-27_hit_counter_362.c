/*
 * TMR-27: Design Hit Counter (LeetCode 362) - hits in the past 5 minutes (300 s)
 *   hit(t): record a hit at second t (t is non-decreasing)
 *   get_hits(t): hits in (t - 300, t]
 * Fixed-memory design: 300 buckets, bucket = t % 300, each stores {time, count}.
 *   hit: if bucket's time != t, the bucket is stale -> reset it to {t, 0}; then count++
 *   get: sum buckets whose time is within the window.  O(300) per get, O(1) per hit,
 *   memory O(300) no matter how many hits (a queue of timestamps would grow without bound).
 * Follow-ups: hits arrive concurrently -> per-bucket atomics or a lock; finer resolution ->
 * more buckets, or keep a running total updated as buckets expire to make get O(1).
 * Embedded link: "packets/errors per second over the last N seconds" counters.
 */
#include <stdio.h>
#include <stdint.h>

#define WINDOW 300

typedef struct { int time[WINDOW]; uint32_t count[WINDOW]; } hit_counter_t;

void hc_init(hit_counter_t *h)
{
    int i;
    for (i = 0; i < WINDOW; i++) { h->time[i] = -WINDOW; h->count[i] = 0; }
}

void hc_hit(hit_counter_t *h, int t)
{
    int b = t % WINDOW;
    if (h->time[b] != t) { h->time[b] = t; h->count[b] = 0; }   /* recycle stale bucket */
    h->count[b]++;
}

uint32_t hc_get(const hit_counter_t *h, int t)
{
    uint32_t sum = 0;
    int i;
    for (i = 0; i < WINDOW; i++)
        if (t - h->time[i] < WINDOW) sum += h->count[i];
    return sum;
}

int main(void)
{
    hit_counter_t h;
    int t;
    struct { int t; uint32_t expect; } q[] = {{4, 3}, {300, 4}, {301, 3}};
    unsigned i;

    hc_init(&h);
    hc_hit(&h, 1); hc_hit(&h, 2); hc_hit(&h, 3);       /* LeetCode example */
    printf("get(4)   = %u (expect 3) %s\n", (unsigned)hc_get(&h, 4), hc_get(&h, 4) == q[0].expect ? "PASS" : "FAIL");
    hc_hit(&h, 300);
    for (i = 1; i < 3; i++)
        printf("get(%d) = %u (expect %u) %s\n", q[i].t, (unsigned)hc_get(&h, q[i].t), (unsigned)q[i].expect,
               hc_get(&h, q[i].t) == q[i].expect ? "PASS" : "FAIL");

    hc_init(&h);
    for (t = 0; t < 1000; t++) { hc_hit(&h, t); hc_hit(&h, t); }   /* 2 hits/s for 1000 s */
    printf("2 hits/s for 1000 s: get(999) = %u (expect 600), memory = %u bytes regardless of hit count\n",
           (unsigned)hc_get(&h, 999), (unsigned)sizeof h);
    return 0;
}
