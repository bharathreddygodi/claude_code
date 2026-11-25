/*
 * CON-02: Counting semaphore built from mutex + condition variable,
 *          then the classic bounded-buffer producer/consumer.
 *   empty_slots starts at N, full_slots starts at 0.
 *   producer: wait(empty) -> lock -> put -> unlock -> signal(full)
 *   consumer: wait(full)  -> lock -> get -> unlock -> signal(empty)
 * Wrong order (lock then wait) = deadlock. Interview: why 'while' not 'if' around
 * cond_wait? -> spurious wakeups / another thread got there first.
 * FreeRTOS: xSemaphoreCreateCounting(max, initial), Take/Give; or just use a queue.
 */
#include <stdio.h>
#include <pthread.h>

typedef struct {
    int count;
    pthread_mutex_t m;
    pthread_cond_t cv;
} sem_t_;

static void sem_init_(sem_t_ *s, int n) { s->count = n; pthread_mutex_init(&s->m, NULL); pthread_cond_init(&s->cv, NULL); }
static void sem_wait_(sem_t_ *s)        /* "take" / P */
{
    pthread_mutex_lock(&s->m);
    while (s->count == 0) pthread_cond_wait(&s->cv, &s->m);
    s->count--;
    pthread_mutex_unlock(&s->m);
}
static void sem_post_(sem_t_ *s)        /* "give" / V */
{
    pthread_mutex_lock(&s->m);
    s->count++;
    pthread_cond_signal(&s->cv);
    pthread_mutex_unlock(&s->m);
}

#define N      4
#define ITEMS 20

static int buf[N], in_idx, out_idx;
static sem_t_ empty_slots, full_slots;
static pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;
static long consumed_sum;

static void *producer(void *arg)
{
    int i;
    (void)arg;
    for (i = 1; i <= ITEMS; i++) {
        sem_wait_(&empty_slots);
        pthread_mutex_lock(&buf_lock);
        buf[in_idx] = i;
        in_idx = (in_idx + 1) % N;
        pthread_mutex_unlock(&buf_lock);
        sem_post_(&full_slots);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    int i, v;
    (void)arg;
    for (i = 0; i < ITEMS; i++) {
        sem_wait_(&full_slots);
        pthread_mutex_lock(&buf_lock);
        v = buf[out_idx];
        out_idx = (out_idx + 1) % N;
        pthread_mutex_unlock(&buf_lock);
        sem_post_(&empty_slots);
        consumed_sum += v;
        printf("%d ", v);
    }
    printf("\n");
    return NULL;
}

int main(void)
{
    pthread_t p, c;
    sem_init_(&empty_slots, N);
    sem_init_(&full_slots, 0);
    printf("consumed in order: ");
    pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    printf("sum = %ld (expect %d) -> %s\n", consumed_sum, ITEMS * (ITEMS + 1) / 2,
           consumed_sum == ITEMS * (ITEMS + 1) / 2 ? "PASS" : "FAIL");
    return 0;
}
