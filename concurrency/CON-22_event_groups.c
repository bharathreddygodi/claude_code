/*
 * CON-22: Event flags / event groups - wait for ANY or ALL of several bits
 * e.g. a "network ready" task waits until WIFI_UP AND IP_ASSIGNED AND TIME_SYNCED.
 * Semaphores signal one event; event groups let one task wait on a combination,
 * and one set_bits can wake many waiting tasks (broadcast).
 * FreeRTOS: xEventGroupSetBits / xEventGroupWaitBits(..., clearOnExit, waitForAll, ...)
 */
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#define WIFI_UP     (1u << 0)
#define IP_ASSIGNED (1u << 1)
#define TIME_SYNCED (1u << 2)
#define BUTTON      (1u << 3)

typedef struct { uint32_t bits; pthread_mutex_t m; pthread_cond_t cv; } event_group_t;

static event_group_t eg = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

static void eg_set(event_group_t *e, uint32_t b)
{
    pthread_mutex_lock(&e->m);
    e->bits |= b;
    pthread_cond_broadcast(&e->cv);           /* wake ALL waiters, each re-checks */
    pthread_mutex_unlock(&e->m);
}

/* clear_mask: bits to clear on exit. Careful: clearing a bit another task is still
   waiting for (e.g. WIFI_UP) makes that task wait forever - a real bug class. */
static uint32_t eg_wait(event_group_t *e, uint32_t want, int wait_all, uint32_t clear_mask)
{
    uint32_t got;
    pthread_mutex_lock(&e->m);
    for (;;) {
        got = e->bits & want;
        if (wait_all ? got == want : got != 0) break;
        pthread_cond_wait(&e->cv, &e->m);
    }
    e->bits &= ~(got & clear_mask);
    pthread_mutex_unlock(&e->m);
    return got;
}

static void *net_task(void *arg)
{
    (void)arg;
    eg_wait(&eg, WIFI_UP | IP_ASSIGNED | TIME_SYNCED, 1, 0);
    printf("  net_task : ALL of WIFI|IP|TIME set -> start MQTT\n");
    return NULL;
}

static void *ui_task(void *arg)
{
    uint32_t got;
    (void)arg;
    got = eg_wait(&eg, BUTTON | WIFI_UP, 0, BUTTON);   /* consume only BUTTON */
    printf("  ui_task  : ANY of BUTTON|WIFI -> woke with 0x%X\n", (unsigned)got);
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, net_task, NULL);
    pthread_create(&b, NULL, ui_task, NULL);
    usleep(50000);
    printf("set BUTTON\n");      eg_set(&eg, BUTTON);      usleep(50000);
    printf("set WIFI_UP\n");     eg_set(&eg, WIFI_UP);     usleep(50000);
    printf("set IP_ASSIGNED\n"); eg_set(&eg, IP_ASSIGNED); usleep(50000);
    printf("set TIME_SYNCED\n"); eg_set(&eg, TIME_SYNCED);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("final bits = 0x%X %s\n", (unsigned)eg.bits,
           (eg.bits & BUTTON) ? "(BUTTON still set)" : "(BUTTON consumed by ui_task)");
    return 0;
}
