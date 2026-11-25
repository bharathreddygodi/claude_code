# Google Senior Embedded / Firmware / Platform — Coding Study Checklist

Tick each item once you can code it from scratch, explain edge cases out loud, and answer the follow-ups.

**Method for every problem:** state assumptions (alignment, ISR/multi-core, malloc allowed?) → simple correct version → test edge cases aloud (0, NULL, all-ones, full/empty, overlap, 64-bit) → optimize.

**IDs:** every item has a permanent ID — `BIT` bit manipulation, `MEM` memory, `CON` concurrency, `DRV` drivers & protocols, `TMR` timers & scheduling. IDs never change when items are reordered; new items get the next free number in their section.

---

## Practiced in Sessions

Questions worked through with Claude, with what to be able to say. Re-do each one cold before ticking it.

- [ ] **CON-09 SPSC ring buffer: UART ISR → main loop** (also MEM-07), `rb_put` / `rb_get`, no RTOS, no IRQ disable (Appendix D)
  - Why lock-free works: one writer per index (ISR writes `head`, main writes `tail`), no shared `count`
  - Full vs empty: sacrifice one slot, *or* free-running indices (`head - tail == SIZE` → full) so all slots are usable
  - Power-of-2 size → `& MASK` instead of `%`, and unsigned wraparound stays correct
  - Single-core: `volatile` indices + compiler barrier (only the compiler reorders)
  - Multi-core: C11 release on publish / acquire on read; `head`/`tail` on separate cache lines (false sharing); bare-metal `__DMB()` equivalent
  - Atomic index size (32-bit on 32-bit MCU); `volatile` ≠ ordering
- [ ] **TMR-13 Software timer module** (also CON-23): `timer_start(ms, cb, arg, periodic)`, `timer_cancel()`, `tick()` from ISR (Appendix G)
  - v1 simple: fixed table + countdown decremented in ISR (O(N) per tick, callbacks in ISR, fixed size, stale ids)
  - v2 improved: ISR only does `ticks++`; sorted list by absolute deadline; callbacks in `timer_run()` from main/task
  - Unlink before callback → cancel/restart self or others safely; periodic re-arm `deadline += period` (no drift)
  - Wraparound-safe compare, late main loop catch-up policy, `timer_next_deadline()` for tickless idle
  - Next: v3 hashed/hierarchical timing wheel for 10k+ timers; start-from-ISR via command queue

---

## 1. Bit Manipulation

### Fundamentals
- [ ] BIT-01 Set / clear / toggle / test bit n — as safe macros (`1U << n`)
- [ ] BIT-02 Power of 2 check: `x && !(x & (x-1))`
- [ ] BIT-03 Popcount — Kernighan, lookup table, SWAR parallel
- [ ] BIT-04 Isolate lowest set bit (`x & -x`) and clear it (`x & (x-1)`)
- [ ] BIT-05 Highest / lowest set bit position (CLZ/CTZ with and without builtins)
- [ ] BIT-06 XOR swap — and why it fails when both point to the same address

### Classic puzzles
- [ ] BIT-07 Reverse bits of a 32-bit int (loop, then O(log n) mask-and-shift)
- [ ] BIT-08 Single number (XOR); variants: others appear 3×, two unique numbers
- [ ] BIT-09 Missing number in 0..n
- [ ] BIT-10 Count bits for 0..n (`bits[i] = bits[i>>1] + (i&1)`)
- [ ] BIT-11 Hamming distance; total Hamming distance across an array
- [ ] BIT-12 Add two integers without `+`
- [ ] BIT-13 Subsets via bitmask; Gray code
- [ ] BIT-14 Next higher number with the same number of set bits

### Embedded / register-level
- [ ] BIT-15 Extract bit field [hi:lo]; write a field without touching other bits (RMW)
- [ ] BIT-16 Generic `GET_FIELD` / `SET_FIELD` with mask + shift
- [ ] BIT-17 Endianness swap 16/32/64-bit; detect endianness at runtime
- [ ] BIT-18 Swap odd/even bits: `((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1)`
- [ ] BIT-19 Swap nibbles in a byte
- [ ] BIT-20 Parity (XOR-fold)
- [ ] BIT-21 Align address up/down: `(addr + a-1) & ~(a-1)`; check pointer alignment
- [ ] BIT-22 Bitmap allocator — find first free, allocate, free
- [ ] BIT-23 Multi-word bitset (`idx >> 5`, `idx & 31`)
- [ ] BIT-24 Sign-extend an N-bit value to 32 bits
- [ ] BIT-25 Rotate left/right without UB (shift by 0 / 32)
- [ ] BIT-26 CRC bit-by-bit, then table-driven

### Follow-up topics
- [ ] BIT-27 Shift UB rules; why `1U << 31` not `1 << 31`
- [ ] BIT-28 `volatile` + non-atomic RMW on registers; set/clear registers (BSRR), bit-banding
- [ ] BIT-29 Why struct bitfields aren't portable for hardware registers
- [ ] BIT-30 Branchless abs / min / max / sign
- [ ] BIT-31 Hardware popcount/CLZ instructions; LUT memory vs speed trade-off

---

## 2. Memory

### Must-code problems
- [x] MEM-01 `aligned_malloc` / `aligned_free` (over-allocate, stash raw pointer) — see Appendix A
- [x] MEM-02 `memmove` with overlap handling — see Appendix B
- [x] MEM-03 Optimized `memcpy` (align head, word copy, unrolled, tail)
- [x] MEM-04 `memset`, `memcmp` efficiently
- [x] MEM-05 Fixed-block memory pool, O(1) free list in a union — see Appendix C
- [x] MEM-06 Simple `malloc` / `free`: headers, first-fit vs best-fit, split, coalesce
- [x] MEM-07 Lock-free SPSC ring buffer (ISR↔task, DMA↔CPU) — same problem as CON-09; see Appendix D
- [x] MEM-08 Stack grows up or down? (and why the naive test is unreliable)
- [x] MEM-09 Bitmap allocator (CTZ on inverted word)
- [ ] MEM-10 Buddy allocator (buddy address via XOR)
- [x] MEM-11 Endian-safe read of `uint32_t` from an unaligned byte buffer (no pointer cast)
- [x] MEM-12 2D array in a single `malloc`
- [x] MEM-13 Implement `realloc` (NULL, size 0, in-place vs copy, failure)
- [x] MEM-14 Guard bytes / canaries to detect overruns on free
- [x] MEM-15 `sizeof` / padding puzzles; reorder fields to minimize size
- [x] MEM-16 Pointer puzzles: `int (*p)[10]` vs `int *p[10]`, `void *` arithmetic, `char *` aliasing

### Concepts to explain
- [ ] MEM-17 `.text` / `.data` / `.bss` / `.rodata` / heap / stack; where a function-local `static int x = 5;` lives
- [ ] MEM-18 Startup code before `main` (copy `.data`, zero `.bss`, set SP, constructors)
- [ ] MEM-19 Linker scripts: placing code in RAM/TCM, LMA vs VMA
- [ ] MEM-20 Unaligned access on Cortex-M0 vs M4/M7; `packed` risks
- [ ] MEM-21 `volatile`, `const volatile`, pointer-to-volatile vs volatile pointer; MMIO access
- [ ] MEM-22 Why ban `malloc` after init; internal vs external fragmentation
- [ ] MEM-23 Leak / double-free / overrun detection on bare metal
- [ ] MEM-24 Stack overflow detection (painting, MPU guard, stack protector); RTOS stack sizing
- [ ] MEM-25 Cache lines, associativity, write-back vs write-through, write-allocate
- [ ] MEM-26 DMA coherency: clean vs invalidate, cache-line-aligned buffers, non-cacheable regions
- [ ] MEM-27 False sharing; row-major vs column-major iteration
- [ ] MEM-28 I-cache issues when running code from RAM
- [ ] MEM-29 DMB / DSB / ISB; compiler vs hardware barriers
- [ ] MEM-30 MPU vs MMU, XN, page tables, TLB; Device vs Normal memory
- [ ] MEM-31 NOR vs NAND, XIP, wear leveling, power-loss-safe storage, A/B update, ECC (SECDED)
- [ ] MEM-32 Debug: random crash after hours; variable changing with no writer; decode HardFault (PC/LR, CFSR/BFAR/MMFAR)

---

## 3. Concurrency

### Build the primitives
- [ ] CON-01 Spinlock with C11 atomics / LDREX-STREX — see Appendix E; why useless on single core
- [ ] CON-02 Counting semaphore from mutex + condvar (`while`, not `if`)
- [ ] CON-03 Mutex from semaphore and vice versa; ownership & priority inheritance
- [ ] CON-04 Reader-writer lock; fix writer starvation
- [ ] CON-05 Reusable barrier (generation counter)
- [ ] CON-06 Thread-safe one-time init / correct double-checked locking
- [ ] CON-07 Atomic increment / CAS with LDREX/STREX; what clears the exclusive monitor

### Producer-consumer
- [ ] CON-08 Bounded blocking queue (mutex + `not_full` / `not_empty`) — most likely question
- [ ] CON-09 Lock-free SPSC ring buffer — single-core (`volatile` + compiler barrier) and multi-core (acquire/release, cache-line padding) versions; see Appendix D
- [ ] CON-10 MPSC / MPMC queue; CAS loops; ABA problem
- [ ] CON-11 RTOS-style message queue with send/receive timeouts
- [ ] CON-12 Ping-pong DMA buffering (half/complete interrupts)

### Thread-ordering puzzles
- [ ] CON-13 Print odd/even with two threads; A-B-C with three threads
- [ ] CON-14 Dining philosophers (show deadlock, fix with ordering)
- [ ] CON-15 H2O / group-before-proceed problems
- [ ] CON-16 Thread pool with work queue and graceful shutdown
- [ ] CON-17 Thread-safe rate limiter / token bucket

### Embedded / ISR-specific
- [ ] CON-18 ISR-safe nestable critical section (save/restore PRIMASK / BASEPRI)
- [ ] CON-19 64-bit counter shared ISR↔main on 32-bit CPU (torn reads; retry loop / seqlock / disable IRQ)
- [ ] CON-20 Seqlock implementation
- [ ] CON-21 Deferred interrupt processing (ISR signals task); why no blocking/mutex in ISR
- [ ] CON-22 Event flag group (wait any / all)
- [ ] CON-23 Software timer service with safe cancellation — same problem as TMR-13; see Section 5 and Appendix G
- [ ] CON-24 Atomic reference counting with decrement-and-free
- [ ] CON-25 Find-and-fix: missing `volatile`, register RMW race, lock taken in ISR

### Concepts
- [ ] CON-26 Priority inversion (Mars Pathfinder), priority inheritance vs ceiling
- [ ] CON-27 Four deadlock conditions; lock ordering, trylock + backoff
- [ ] CON-28 Livelock vs starvation
- [ ] CON-29 Memory orders (relaxed / acquire / release / seq_cst); `volatile` ≠ atomic
- [ ] CON-30 False sharing and cache-line padding
- [ ] CON-31 Spinlock vs mutex vs semaphore vs IRQ-disable trade-offs
- [ ] CON-32 Multi-core SoC: IPIs, hardware semaphores, mailboxes, shared memory + cache maintenance

---

## 4. Drivers & Protocols

### Peripheral drivers
- [ ] DRV-01 Interrupt-driven UART with TX/RX ring buffers — most likely question
- [ ] DRV-02 I2C `read_reg` / `write_reg` with repeated start; NACK, arbitration loss, timeouts
- [ ] DRV-03 I2C bus recovery (9 SCL clocks + STOP)
- [ ] DRV-04 SPI full-duplex transfer; CS handling; CPOL/CPHA modes; shared bus
- [ ] DRV-05 Bit-banged I2C / SPI
- [ ] DRV-06 GPIO with debounce (timer + state machine, edge IRQ)
- [ ] DRV-07 Timer/PWM prescaler & reload math; input-capture frequency measurement
- [ ] DRV-08 ADC + circular DMA with half/complete processing
- [ ] DRV-09 Watchdog supervisor with per-task heartbeats

### Driver API / HAL design
- [ ] DRV-10 Generic sensor driver via `ops` struct of function pointers
- [ ] DRV-11 Register struct overlay vs offset macros
- [ ] DRV-12 Sync vs async APIs (callbacks, completion semaphores)
- [ ] DRV-13 Portability & host-side unit testing (HAL mocking)
- [ ] DRV-14 Suspend/resume, clock gating, wake sources

### Protocol parsing & framing
- [ ] DRV-15 Byte-at-a-time packet parser state machine with resync — see Appendix F
- [ ] DRV-16 Add inter-byte timeout, 16-bit length, SOF inside payload
- [ ] DRV-17 SLIP / HDLC / COBS encode & decode
- [ ] DRV-18 CRC-8/16/32 (polynomial, init, reflection)
- [ ] DRV-19 Endian-safe, padding-safe struct serialize/deserialize
- [ ] DRV-20 TLV parser robust to unknown types and bad lengths
- [ ] DRV-21 Reliable protocol over lossy link (seq numbers, ACK/NACK, retransmit, sliding window)
- [ ] DRV-22 Firmware update protocol: chunks, per-block CRC, resume after power loss, A/B slots

### Linux / platform drivers
- [ ] DRV-23 Char device skeleton: `open/read/write/ioctl`, `copy_to/from_user`
- [ ] DRV-24 Platform driver + device tree `probe` (IRQs, regs, clocks)
- [ ] DRV-25 Top half vs bottom half (tasklet, workqueue, threaded IRQ)
- [ ] DRV-26 NIC-style DMA descriptor ring (ownership bits, head/tail, coherency)
- [ ] DRV-27 Kernel locking: `spin_lock_irqsave`, no sleeping under spinlock

### Bus knowledge & debugging
- [ ] DRV-28 I2C vs SPI vs UART; clock stretching; pull-up sizing
- [ ] DRV-29 CAN arbitration, error frames, bus-off, CAN FD
- [ ] DRV-30 USB enumeration, descriptors, endpoint types, CDC vs HID
- [ ] DRV-31 PCIe / MIPI basics (BARs, MSI, descriptor rings)
- [ ] DRV-32 Debug: I2C returns 0xFF; UART drops bytes at high baud; SPI works on bench but fails in product; logic analyzer / scope usage

---

## 5. Timers & Scheduling

### Concepts to explain
- [ ] TMR-01 Time base: periodic tick (SysTick) vs free-running counter + compare match
- [ ] TMR-02 Tick vs tickless idle (program compare for the next deadline, sleep — power trade-off)
- [ ] TMR-03 Wraparound-safe compare: `(int32_t)(now - deadline) >= 0`; valid while delta < 2^31 (Linux `time_after()`)
- [ ] TMR-04 Safe 64-bit read from two 32-bit registers (read hi, lo, re-read hi; retry if changed)
- [ ] TMR-05 Software timer data structures: unsorted array / sorted (delta) list / min-heap / hierarchical timing wheel — insert & expiry cost
- [ ] TMR-06 Callback context: in ISR vs deferred to a timer task (FreeRTOS timer service task)
- [ ] TMR-07 Periodic drift: `next = prev_deadline + period`, not `now + period`; catch-up vs skip policy when late
- [ ] TMR-08 Superloop → cooperative (run-to-completion, time-triggered) → preemptive priority RTOS; context switch basics
- [ ] TMR-09 Rate-monotonic scheduling; utilization bound n(2^(1/n) − 1) → ~69%; EDF up to 100%
- [ ] TMR-10 Priority inversion and fixes (inheritance / ceiling)
- [ ] TMR-11 Token bucket vs leaky bucket rate limiting
- [ ] TMR-12 Timeouts and retry with exponential backoff

### Must-code problems
- [ ] TMR-13 Software timer module: `timer_start` / `timer_cancel` / `tick` — v1 simple, v2 improved (Appendix G); follow-ups: 10k timers, cancel inside callback, ISR safety
- [ ] TMR-14 `time_after()` with wraparound — explain why the signed cast works and where it breaks
- [ ] TMR-15 Safe 64-bit counter read from two 32-bit registers
- [ ] TMR-16 `delay_ms()` / timeout loop that survives wraparound
- [ ] TMR-17 Cooperative scheduler: task table `{period, next_run, fn}`; task overrun, missed deadlines, priorities
- [ ] TMR-18 Tickless idle: compute earliest deadline, program compare, sleep
- [ ] TMR-19 Button debounce with timer + state machine
- [ ] TMR-20 Task watchdog: each task checks in, kick HW watchdog only if all did
- [ ] TMR-21 Schedulability check given periods + WCETs (RMS)
- [ ] TMR-22 Hashed / hierarchical timing wheel (O(1) insert, cancel, expiry) — "1M connections with idle timeouts"
- [ ] TMR-23 `timer_start()` from ISR via command queue drained by the timer task (cf. `xTimerStartFromISR`)

### LeetCode-style (general coding rounds)
- [ ] TMR-24 Task Scheduler (621)
- [ ] TMR-25 Meeting Rooms II (253) — min-heap
- [ ] TMR-26 Logger Rate Limiter (359)
- [ ] TMR-27 Design Hit Counter (362)

### Senior signals
- [ ] TMR-28 Say which context each function runs in (ISR vs task), state per-tick complexity, raise wraparound before being asked, discuss tick vs tickless power trade-off

---

## Appendix — Reference Solutions

### A. aligned_malloc / aligned_free — MEM-01
```c
void *aligned_malloc(size_t size, size_t align) {
    if (align == 0 || (align & (align - 1))) return NULL;
    void *raw = malloc(size + align - 1 + sizeof(void *));
    if (!raw) return NULL;
    uintptr_t start   = (uintptr_t)raw + sizeof(void *);
    uintptr_t aligned = (start + align - 1) & ~(uintptr_t)(align - 1);
    ((void **)aligned)[-1] = raw;
    return (void *)aligned;
}
void aligned_free(void *p) { if (p) free(((void **)p)[-1]); }
```
Follow-ups: size overflow check, worst-case waste, `aligned_realloc`, DMA cache-line alignment.

### B. memmove — MEM-02
```c
void *my_memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst; const unsigned char *s = src;
    if (d == s || n == 0) return dst;
    if ((uintptr_t)d < (uintptr_t)s) { while (n--) *d++ = *s++; }
    else { d += n; s += n; while (n--) *--d = *--s; }
    return dst;
}
```
Follow-ups: word-sized optimization, mismatched alignment, why compare as `uintptr_t`.

### C. Fixed-block pool — MEM-05
```c
typedef union block { union block *next; uint8_t data[BLOCK_SIZE]; } block_t;
static block_t pool[NUM_BLOCKS];
static block_t *free_list;

void pool_init(void) {
    for (int i = 0; i < NUM_BLOCKS - 1; i++) pool[i].next = &pool[i + 1];
    pool[NUM_BLOCKS - 1].next = NULL;
    free_list = pool;
}
void *pool_alloc(void) { block_t *b = free_list; if (b) free_list = b->next; return b; }
void pool_free(void *p) { block_t *b = p; b->next = free_list; free_list = b; }
```
Follow-ups: ISR/thread safety, ABA, double-free detection, multiple size classes.

### D. Lock-free SPSC ring buffer — CON-09 / MEM-07

**Multi-core (C11 acquire/release)**
```c
#define RB_SIZE 256  /* power of 2 */
typedef struct {
    _Alignas(64) _Atomic uint32_t head;   /* producer-owned; own cache line */
    _Alignas(64) _Atomic uint32_t tail;   /* consumer-owned */
    uint8_t buf[RB_SIZE];
} rb_t;

bool rb_put(rb_t *r, uint8_t v) {
    uint32_t h = atomic_load_explicit(&r->head, memory_order_relaxed);
    uint32_t t = atomic_load_explicit(&r->tail, memory_order_acquire);
    if (h - t == RB_SIZE) return false;
    r->buf[h & (RB_SIZE - 1)] = v;
    atomic_store_explicit(&r->head, h + 1, memory_order_release);
    return true;
}
bool rb_get(rb_t *r, uint8_t *v) {
    uint32_t t = atomic_load_explicit(&r->tail, memory_order_relaxed);
    uint32_t h = atomic_load_explicit(&r->head, memory_order_acquire);
    if (h == t) return false;
    *v = r->buf[t & (RB_SIZE - 1)];
    atomic_store_explicit(&r->tail, t + 1, memory_order_release);
    return true;
}
```

**Single-core MCU (ISR producer → main-loop consumer)**
```c
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
typedef struct { volatile uint32_t head, tail; uint8_t buf[RB_SIZE]; } rb_sc_t;

bool rb_sc_put(rb_sc_t *r, uint8_t v) {          /* ISR */
    uint32_t h = r->head, t = r->tail;
    if (h - t == RB_SIZE) return false;
    r->buf[h & (RB_SIZE - 1)] = v;
    COMPILER_BARRIER();                           /* data before publish */
    r->head = h + 1;
    return true;
}
bool rb_sc_get(rb_sc_t *r, uint8_t *v) {         /* main loop */
    uint32_t t = r->tail, h = r->head;
    if (h == t) return false;
    COMPILER_BARRIER();                           /* index before data */
    *v = r->buf[t & (RB_SIZE - 1)];
    COMPILER_BARRIER();                           /* read done before freeing slot */
    r->tail = t + 1;
    return true;
}
```
Bare-metal multi-core without C11: `__DMB()` before the `head` store in put, and after the `head` load (and before the `tail` store) in get.

Follow-ups: free-running indices vs sacrificed slot, acquire/release vs `volatile`, false sharing, multi-producer, 8/16-bit MCU index atomicity.

### E. Spinlock — CON-01
```c
atomic_flag lock = ATOMIC_FLAG_INIT;
void spin_lock(void)   { while (atomic_flag_test_and_set_explicit(&lock, memory_order_acquire)) ; }
void spin_unlock(void) { atomic_flag_clear_explicit(&lock, memory_order_release); }
```

### F. Packet parser state machine — DRV-15
```c
typedef enum { WAIT_SOF, GET_LEN, GET_PAYLOAD, GET_CRC } state_t;
void parser_feed(parser_t *p, uint8_t b) {
    switch (p->state) {
    case WAIT_SOF:    if (b == SOF) p->state = GET_LEN; break;
    case GET_LEN:     if (b == 0 || b > MAX_LEN) { p->state = WAIT_SOF; break; }
                      p->len = b; p->idx = 0; p->state = GET_PAYLOAD; break;
    case GET_PAYLOAD: p->buf[p->idx++] = b;
                      if (p->idx == p->len) p->state = GET_CRC; break;
    case GET_CRC:     if (b == crc8(p->buf, p->len)) deliver(p->buf, p->len);
                      p->state = WAIT_SOF; break;
    }
}
```

### G. Software timer module — TMR-13 / CON-23

**v1 — simple (fixed table, countdown in ISR)**
```c
typedef struct { volatile bool active; uint32_t remaining, period; timer_cb_t cb; void *arg; } sw_timer_t;
static sw_timer_t g_timers[MAX_TIMERS];

void timer_tick(void) {                    /* 1 ms ISR: O(MAX_TIMERS) every tick */
    for (int i = 0; i < MAX_TIMERS; i++) {
        sw_timer_t *t = &g_timers[i];
        if (t->active && --t->remaining == 0) {
            if (t->period) t->remaining = t->period; else t->active = false;
            t->cb(t->arg);                 /* runs in ISR context */
        }
    }
}
```
Weaknesses: O(N) per tick, callbacks in ISR, fixed size, stale integer ids (ABA), start/cancel need IRQ masking.

**v2 — improved (ISR = `ticks++`, sorted list, callbacks in task context)**
```c
typedef struct sw_timer {
    struct sw_timer *prev, *next;
    uint32_t deadline, period; timer_cb_t cb; void *arg;
    bool armed, linked;
} sw_timer_t;                                /* caller-owned: no malloc, no limit */

static volatile uint32_t g_ticks;
static sw_timer_t *g_head;                   /* sorted by deadline */
static inline bool time_before(uint32_t a, uint32_t b) { return (int32_t)(a - b) < 0; }

void timer_isr_tick(void) { g_ticks++; }     /* O(1) ISR */

void timer_start(sw_timer_t *t, uint32_t ms, bool periodic) {
    if (t->linked) list_remove(t);           /* restart semantics */
    t->deadline = g_ticks + ms_to_ticks(ms);
    t->period   = periodic ? ms_to_ticks(ms) : 0;
    t->armed    = true;
    list_insert(t);                          /* O(n), FIFO for equal deadlines */
}
void timer_cancel(sw_timer_t *t) { t->armed = false; if (t->linked) list_remove(t); }

void timer_run(void) {                       /* main loop / timer task */
    uint32_t now = g_ticks;
    while (g_head && !time_before(now, g_head->deadline)) {
        sw_timer_t *t = g_head;
        list_remove(t);                      /* unlink BEFORE callback */
        if (!t->period) t->armed = false;
        t->cb(t->arg);                       /* may cancel/restart anything */
        if (t->period && t->armed && !t->linked) {
            t->deadline += t->period;        /* no drift */
            list_insert(t);
        }
    }
}
```
Test cases to walk through: one-shot timing, periodic + cancel, ordering with equal deadlines, cancel self in callback, cancel another timer due on the same tick, one-shot restarting itself, wraparound near `UINT32_MAX`, late main loop catch-up, `timer_next_deadline()` for tickless.

Next (v3): hashed / hierarchical timing wheel for O(1) at 10k+ timers; start-from-ISR via command queue.
