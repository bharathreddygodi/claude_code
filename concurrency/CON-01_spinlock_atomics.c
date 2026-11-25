/*
 * CON-01: Atomics and spinlocks (C11 <stdatomic.h>)
 *  - atomic_fetch_add: lock-free counter (LDREX/STREX loop on ARM, LOCK XADD on x86)
 *  - spinlock with atomic_flag test-and-set: busy-waits instead of sleeping
 *    -> fine on multi-core for very short sections; on a SINGLE core it is useless
 *       (the holder can't run while you spin) -> disable interrupts instead.
 *  - compare-and-swap (CAS) loop: building block for lock-free structures
 *  - memory ordering: acquire on lock, release on unlock
 * Interview: volatile vs atomic: volatile = no caching, atomic = indivisible + ordering.
 */
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#define ITER 500000

static atomic_int atomic_counter;
static int spin_counter;
static atomic_flag spin = ATOMIC_FLAG_INIT;
static atomic_int max_seen;

static void spin_lock(void)   { while (atomic_flag_test_and_set_explicit(&spin, memory_order_acquire)) { } }
static void spin_unlock(void) { atomic_flag_clear_explicit(&spin, memory_order_release); }
// Following are written by Bharath to show the other way to implement
static void spin_lock_v2(void)   { while (atomic_flag_test_and_set(&spin)) { } }
static void spin_unlock_v2(void)   { atomic_flag_clear(&spin); }

static void atomic_max(atomic_int *m, int v)      /* CAS loop */
{
    int cur = atomic_load(m);
    while (v > cur && !atomic_compare_exchange_weak(m, &cur, v)) { }   /* cur reloaded on fail */
}

static void *worker(void *arg)
{
    int i, base = *(int *)arg;
    for (i = 0; i < ITER; i++) {
        atomic_fetch_add(&atomic_counter, 1);
        spin_lock();
        spin_counter++;
        spin_unlock();
        if (i % 1000 == 0) atomic_max(&max_seen, base + i);
    }
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    int base[4] = {0, 1, 2, 3}, i;
    for (i = 0; i < 4; i++) pthread_create(&t[i], NULL, worker, &base[i]);
    for (i = 0; i < 4; i++) pthread_join(t[i], NULL);

    printf("atomic counter  = %d (expect %d) %s\n", atomic_load(&atomic_counter), 4 * ITER,
           atomic_load(&atomic_counter) == 4 * ITER ? "PASS" : "FAIL");
    printf("spinlock counter= %d (expect %d) %s\n", spin_counter, 4 * ITER, spin_counter == 4 * ITER ? "PASS" : "FAIL");
    printf("CAS max         = %d (expect %d)\n", atomic_load(&max_seen), 3 + ITER - 1000);
    return 0;
}
