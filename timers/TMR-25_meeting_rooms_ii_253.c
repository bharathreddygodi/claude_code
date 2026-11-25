/*
 * TMR-25: Meeting Rooms II (LeetCode 253) - minimum rooms for overlapping intervals
 * Method 1 (min-heap, O(n log n)): sort meetings by start. Keep a min-heap of end times
 *   of rooms in use. For each meeting: if the earliest-ending room is free by now
 *   (heap top <= start) reuse it (pop), then push this meeting's end. Rooms = heap size.
 * Method 2 (sweep, O(n log n)): sort starts and ends separately; walk both; a start
 *   before the next end needs a new room, otherwise one frees up.
 * Embedded link: "how many DMA channels / timers / buffers do I need at peak?" -
 * same as the maximum number of overlapping intervals.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct { int start, end; } iv_t;

/* --- small array-based min-heap --- */
static int heap[64], hn;
static void heap_push(int v)
{
    int i = hn++;
    heap[i] = v;
    while (i > 0 && heap[(i - 1) / 2] > heap[i]) { int p = (i - 1) / 2, t = heap[p]; heap[p] = heap[i]; heap[i] = t; i = p; }
}
static void heap_pop(void)
{
    int i = 0;
    heap[0] = heap[--hn];
    for (;;) {
        int l = 2 * i + 1, r = l + 1, m = i, t;
        if (l < hn && heap[l] < heap[m]) m = l;
        if (r < hn && heap[r] < heap[m]) m = r;
        if (m == i) break;
        t = heap[m]; heap[m] = heap[i]; heap[i] = t; i = m;
    }
}

static int cmp_start(const void *a, const void *b) { return ((const iv_t *)a)->start - ((const iv_t *)b)->start; }
static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

int rooms_heap(iv_t *v, int n)
{
    int i, best = 0;
    qsort(v, (size_t)n, sizeof *v, cmp_start);
    hn = 0;
    for (i = 0; i < n; i++) {
        if (hn && heap[0] <= v[i].start) heap_pop();   /* a room freed up; end == start is fine */
        heap_push(v[i].end);
        if (hn > best) best = hn;
    }
    return best;
}

int rooms_sweep(const iv_t *v, int n)
{
    int s[64], e[64], i, j = 0, rooms = 0, best = 0;
    for (i = 0; i < n; i++) { s[i] = v[i].start; e[i] = v[i].end; }
    qsort(s, (size_t)n, sizeof(int), cmp_int);
    qsort(e, (size_t)n, sizeof(int), cmp_int);
    for (i = 0; i < n; i++) {
        if (s[i] < e[j]) rooms++; else j++;            /* new room, or reuse one that ended */
        if (rooms > best) best = rooms;
    }
    return best;
}

int main(void)
{
    iv_t a[] = {{0, 30}, {5, 10}, {15, 20}};
    iv_t b[] = {{7, 10}, {2, 4}};
    iv_t c[] = {{1, 5}, {5, 10}, {10, 15}};                 /* back-to-back: 1 room */
    iv_t d[] = {{1, 10}, {2, 9}, {3, 8}, {4, 7}, {11, 12}}; /* 4 nested */
    struct { iv_t *v; int n, expect; } tc[] = {{a, 3, 2}, {b, 2, 1}, {c, 3, 1}, {d, 5, 4}};
    unsigned i;
    for (i = 0; i < 4; i++) {
        int s = rooms_sweep(tc[i].v, tc[i].n), h = rooms_heap(tc[i].v, tc[i].n);
        printf("case %u: heap=%d sweep=%d expect=%d %s\n", i, h, s, tc[i].expect,
               (h == tc[i].expect && s == tc[i].expect) ? "PASS" : "FAIL");
    }
    return 0;
}
