/*
 * CON-08: Blocking message queue (fixed-size messages copied by value)
 *   send: block (with timeout) while full; receive: block while empty.
 * This is the #1 way tasks talk in an RTOS (FreeRTOS xQueueSend / xQueueReceive,
 * xQueueSendFromISR from interrupts - ISR versions never block).
 * Copy-by-value means the sender's buffer can be reused immediately; for big data
 * send a pointer to a pool buffer instead.
 * Here: an "ISR" thread pushes sensor samples, a task thread processes them.
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define QLEN 4

typedef struct { int id; int value; } msg_t;

typedef struct {
    msg_t buf[QLEN];
    int head, tail, count;
    pthread_mutex_t m;
    pthread_cond_t not_empty, not_full;
} queue_t;

static void q_init(queue_t *q)
{
    memset(q, 0, sizeof *q);
    pthread_mutex_init(&q->m, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

static void q_send(queue_t *q, const msg_t *msg)
{
    pthread_mutex_lock(&q->m);
    while (q->count == QLEN) pthread_cond_wait(&q->not_full, &q->m);
    q->buf[q->tail] = *msg;                   /* copy in */
    q->tail = (q->tail + 1) % QLEN;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->m);
}

static int q_receive(queue_t *q, msg_t *out, int timeout_ms)   /* 0 ok, -1 timeout */
{
    struct timespec ts;
    int rc = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }

    pthread_mutex_lock(&q->m);
    while (q->count == 0 && rc != ETIMEDOUT)
        rc = pthread_cond_timedwait(&q->not_empty, &q->m, &ts);
    if (q->count == 0) { pthread_mutex_unlock(&q->m); return -1; }
    *out = q->buf[q->head];                   /* copy out */
    q->head = (q->head + 1) % QLEN;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->m);
    return 0;
}

static queue_t q;

static void *sensor_isr_sim(void *arg)
{
    int i;
    (void)arg;
    for (i = 1; i <= 8; i++) {
        msg_t m = {i, i * 10};
        q_send(&q, &m);
    }
    return NULL;
}

int main(void)
{
    pthread_t prod;
    msg_t m;
    int total = 0;

    q_init(&q);
    pthread_create(&prod, NULL, sensor_isr_sim, NULL);
    while (q_receive(&q, &m, 200) == 0) {
        printf("task got msg id=%d value=%d\n", m.id, m.value);
        total += m.value;
    }
    pthread_join(prod, NULL);
    printf("receive timed out (queue empty) -> total=%d %s\n", total, total == 360 ? "PASS" : "FAIL");
    return 0;
}
