/*
 * MEM-07: Circular (ring) buffer - UART/ISR producer, main-loop consumer
 * - Power-of-2 size so wrap-around is a cheap mask instead of a modulo.
 * - head/tail are free-running counters: count = head - tail (unsigned wrap is safe),
 *   so all N slots are usable (no "waste one slot" trick needed).
 * - Single producer / single consumer is lock-free: ISR only writes head,
 *   main only writes tail; mark them volatile (and use barriers on multi-core).
 * Interview points: full vs empty detection, overwrite-oldest vs reject policy,
 * why volatile, race conditions, memory barriers.
 */
#include <stdio.h>
#include <stdint.h>

#define RB_SIZE 8u                       /* must be a power of 2 */
#define RB_MASK (RB_SIZE - 1u)

typedef struct {
    uint8_t buf[RB_SIZE];
    volatile uint32_t head;              /* written by producer */
    volatile uint32_t tail;              /* written by consumer */
} ring_t;

static void     rb_init(ring_t *r)        { r->head = r->tail = 0; }
static uint32_t rb_count(const ring_t *r) { return r->head - r->tail; }
static int      rb_empty(const ring_t *r) { return r->head == r->tail; }
static int      rb_full(const ring_t *r)  { return rb_count(r) == RB_SIZE; }

int rb_put(ring_t *r, uint8_t v)         /* returns 0 ok, -1 full */
{
    if (rb_full(r))
        return -1;
    r->buf[r->head & RB_MASK] = v;
    r->head++;                           /* publish AFTER writing data */
    return 0;
}

int rb_get(ring_t *r, uint8_t *v)        /* returns 0 ok, -1 empty */
{
    if (rb_empty(r))
        return -1;
    *v = r->buf[r->tail & RB_MASK];
    r->tail++;
    return 0;
}

void rb_put_overwrite(ring_t *r, uint8_t v)   /* logger style: drop oldest */
{
    if (rb_full(r))
        r->tail++;
    r->buf[r->head & RB_MASK] = v;
    r->head++;
}

int main(void)
{
    ring_t r;
    uint8_t v = 0, i;

    rb_init(&r);
    for (i = 0; i < 10; i++)
        if (rb_put(&r, i) < 0)
            printf("put %u -> FULL (rejected)\n", i);
    printf("count=%u full=%d\n", (unsigned)rb_count(&r), rb_full(&r));

    printf("get 3: ");
    for (i = 0; i < 3; i++) { rb_get(&r, &v); printf("%u ", v); }
    printf("\n");

    for (i = 100; i < 103; i++)          /* wraps around the array end */
        rb_put(&r, i);
    printf("drain: ");
    while (rb_get(&r, &v) == 0)
        printf("%u ", v);
    printf("\nempty=%d\n", rb_empty(&r));

    for (i = 0; i < 12; i++)
        rb_put_overwrite(&r, i);
    printf("overwrite mode keeps newest 8: ");
    while (rb_get(&r, &v) == 0)
        printf("%u ", v);
    printf("\n");

    r.head = r.tail = 0xFFFFFFFEu;       /* counter wrap-around test */
    for (i = 0; i < 4; i++) rb_put(&r, i);
    printf("counter wrap: count=%u -> %s\n", (unsigned)rb_count(&r), rb_count(&r) == 4 ? "PASS" : "FAIL");
    return 0;
}
