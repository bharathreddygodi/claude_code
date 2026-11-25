/*
 * CON-27: Deadlock and how to avoid it
 * Four Coffman conditions (all must hold): mutual exclusion, hold-and-wait,
 * no preemption, circular wait. Break any one:
 *   1) Lock ordering - always take A before B (breaks circular wait)  <- best answer
 *   2) Try-lock with timeout, release everything and retry (breaks hold-and-wait)
 *   3) Take all resources at once / single coarse lock
 * The deadlocking version is NOT run (it would hang); it is described in comments:
 *   task1: lock(A); lock(B)     task2: lock(B); lock(A)   -> each waits forever.
 */
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER, B = PTHREAD_MUTEX_INITIALIZER;
static int shared_a, shared_b;

/* Fix 1: both tasks use the same global order A -> B */
static void *ordered_task(void *arg)
{
    int i, id = *(int *)arg;
    for (i = 0; i < 100000; i++) {
        pthread_mutex_lock(&A);
        pthread_mutex_lock(&B);
        shared_a++; shared_b += id;
        pthread_mutex_unlock(&B);
        pthread_mutex_unlock(&A);
    }
    return NULL;
}

/* Fix 2: task takes in "wrong" order but backs off if second lock is busy */
static int backoffs;
static void *trylock_task(void *arg)
{
    int i;
    (void)arg;
    for (i = 0; i < 100000; i++) {
        for (;;) {
            pthread_mutex_lock(&B);
            if (pthread_mutex_trylock(&A) == 0) break;  /* got both */
            pthread_mutex_unlock(&B);                   /* release and retry */
            backoffs++;
            sched_yield();
        }
        shared_a++; shared_b++;
        pthread_mutex_unlock(&A);
        pthread_mutex_unlock(&B);
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, NULL, ordered_task, &id1);
    pthread_create(&t2, NULL, ordered_task, &id2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("lock ordering : finished, shared_a=%d -> PASS (no deadlock)\n", shared_a);

    shared_a = shared_b = 0;
    pthread_create(&t1, NULL, ordered_task, &id1);   /* A then B */
    pthread_create(&t2, NULL, trylock_task, NULL);   /* B then try A */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("trylock+backoff: finished, shared_a=%d, backoffs=%d -> PASS\n", shared_a, backoffs);
    return 0;
}
