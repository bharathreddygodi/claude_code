/*
 * RTOS-01: Race condition and mutex (critical section)
 * counter++ is NOT atomic: load -> add -> store. Two tasks interleaving lose updates.
 * Fixes: mutex (tasks), disable interrupts (task vs ISR, very short sections),
 * atomic operations (single variables).
 *
 * Mutex vs binary semaphore (top interview question):
 *   mutex     : has an OWNER, only the owner unlocks, supports priority inheritance,
 *               can be recursive; used for mutual exclusion. Never from an ISR.
 *   binary sem: no owner, anyone can give; used for SIGNALLING (ISR -> task).
 * pthreads stand in for RTOS tasks here; FreeRTOS equivalents in comments.
 */
#include <stdio.h>
#include <pthread.h>

#define ITER 1000000

static volatile long unsafe_counter;   /* volatile does NOT make it safe */
static long safe_counter;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;   /* xSemaphoreCreateMutex() */

static void *unsafe_task(void *arg)
{
    int i;
    (void)arg;
    for (i = 0; i < ITER; i++) unsafe_counter++;
    return NULL;
}

static void *safe_task(void *arg)
{
    int i;
    (void)arg;
    for (i = 0; i < ITER; i++) {
        pthread_mutex_lock(&lock);       /* xSemaphoreTake(mutex, portMAX_DELAY) */
        safe_counter++;                  /* critical section: keep it SHORT */
        pthread_mutex_unlock(&lock);     /* xSemaphoreGive(mutex) */
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;

    pthread_create(&a, NULL, unsafe_task, NULL);
    pthread_create(&b, NULL, unsafe_task, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("without mutex: %ld (expected %d) -> %s\n", unsafe_counter, 2 * ITER,
           unsafe_counter == 2 * ITER ? "got lucky" : "LOST UPDATES (race)");

    pthread_create(&a, NULL, safe_task, NULL);
    pthread_create(&b, NULL, safe_task, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("with mutex   : %ld (expected %d) -> %s\n", safe_counter, 2 * ITER,
           safe_counter == 2 * ITER ? "PASS" : "FAIL");
    return 0;
}
