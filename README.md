# C / Embedded Interview Prep

| Folder | Problems |
|--------|----------|
| [memory/](memory/README.md) | 15 files numbered to match the Google checklist (MEM-01 .. MEM-16, MEM-09 is bits/BIT-22) + 3 extras |
| [bits/](bits/) | 19 files, numbered to match the Google checklist (BIT-01 .. BIT-26) |
| [strings/](strings/) | STR-01 .. STR-12 |
| [pointers/](pointers/) | PTR-01 .. PTR-10 |
| [linked_lists/](linked_lists/) | LL-01 .. LL-12 |
| [rtos/](rtos/) | RTOS-01, 05, 06, 07, 10 (the rest moved to concurrency/) |
| [interrupts/](interrupts/) | INT-02, 03, 05 .. 09 (INT-01, 04 moved to concurrency/) |
| [drivers/](drivers/) | DRV-01 .. 07, 09, 10 (DRV-08 moved to concurrency/) |
| [concurrency/](concurrency/) | CON-01 .. CON-27 that were already solved, numbered to match the Google checklist |
| [timers/](timers/) | TMR-13 .. TMR-27, numbered to match the Google checklist |

Run: `sh run.sh` (all) · `sh run.sh bits` (one folder) · `sh run.sh bits BIT-05` (one problem)
Every file is standalone with its own `main()` and PASS/FAIL checks; the header comment
lists the talking points.

## Bits
Numbered to match "Google Embedded Interview - Coding Study Checklist.md".
| # | Topic | Also covers | Key idea |
|---|-------|-------------|----------|
| 01 | set / clear / toggle / check, register field R-M-W | 16 (GET/SET_FIELD) | `1UL << n`, mask = `((1<<w)-1) << pos` |
| 02 | power of 2, next power of 2 | | `x && !(x & (x-1))`, smear bits then +1 |
| 03 | count set bits | | Kernighan `x &= x-1`, SWAR, `__builtin_popcount` |
| 05 | lowest / highest set bit position | 04 (isolate/clear lowest) | `x & -x`, ctz/clz, undefined for 0 |
| 06 | XOR swap | 18 (odd/even), 19 (nibbles) | XOR swap breaks when `a == b` alias |
| 07 | reverse bits | | swap halves -> bytes -> nibbles -> pairs -> bits |
| 08 | single number, two singles, others-appear-3x | 09 (missing number) | `a^a = 0`; 3x: bit count mod 3 or ones/twos |
| 10 | count bits for 0..n | | `bits[i] = bits[i>>1] + (i&1)`; builds a popcount LUT |
| 11 | Hamming distance, total over array | | `popcount(a^b)`; per bit `k*(n-k)` |
| 12 | add without + | 30 (branchless abs/min/max/sign) | carry = `(a & b) << 1` |
| 13 | subsets via bitmask, Gray code | | `s = (s-1) & mask` walks submasks; `i ^ (i>>1)` |
| 14 | next higher number, same popcount | | Gosper: `c = x&-x; r = x+c; r \| (((r^x)>>2)/c)` |
| 15 | extract / insert bit range | | `(1 << 32)` is UB - guard full width |
| 20 | parity | 13 (Gray decode) | XOR fold |
| 21 | align address up/down, pointer alignment | | `(x + a-1) & ~(a-1)`; math on `uintptr_t` |
| 22 | bitmap allocator: alloc, contiguous run, free | | `ctz(~word)`; catch double free |
| 23 | multi-word bitset | | word `n >> 5`, bit `n & 31` |
| 24 | sign-extend N-bit to 32-bit | | `m = 1<<(n-1); (x ^ m) - m` |
| 25 | rotate left / right | | `(-n & 31)` to avoid shift-by-32 UB |

Checklist items solved in other folders: BIT-17 endianness -> memory/endianness_check, MEM-11 ·
BIT-26 CRC -> drivers/DRV-05 · BIT-28 BSRR / volatile RMW -> drivers/DRV-01, pointers/PTR-08 ·
BIT-29 bitfields -> memory/MEM-15.
BIT-01 to BIT-26 are all covered. Items 04, 09, 16, 18 and 19 live inside another file (see "Also covers").

## Strings
| # | Topic | Key idea |
|---|-------|----------|
| 01 | strlen / strcpy / strncpy / strcat / strlcpy | strncpy may not terminate |
| 02 | strcmp / strncmp / strcasecmp | compare as `unsigned char` |
| 03 | reverse string, reverse words | reverse all, then each word |
| 04 | palindrome (string, cleaned, integer, longest) | two pointers; expand around centre |
| 05 | atoi / itoa | whitespace, sign, overflow clamp, INT_MIN |
| 06 | strstr + KMP | brute force O(nm), KMP O(n+m) |
| 07 | strtok / strtok_r | static state = not reentrant |
| 08 | first unique, remove dups, all unique | 256-entry count table |
| 09 | anagram, rotation | count diff; `b` in `a+a` |
| 10 | trim, remove char, collapse spaces | read ptr + write ptr in place |
| 11 | run-length encoding | multi-digit counts, bounds check |
| 12 | hex string <-> bytes, hex dump | validate chars and odd length |

## Pointers
| # | Topic | Key idea |
|---|-------|----------|
| 01 | pointer arithmetic, `*p++` vs `(*p)++` | `p+1` moves `sizeof(*p)` bytes |
| 02 | const pointer variants | read right to left |
| 03 | pointer to pointer, list insert/delete | change caller's pointer needs `T**` |
| 04 | function pointers, command table, FSM | table of handlers |
| 05 | void* generic swap, qsort comparator | `(x>y)-(x<y)`, not `x-y` |
| 06 | array vs pointer, `&arr` vs `arr` | `sizeof` of array param = pointer size |
| 07 | complex declarations | right-left rule, typedef `signal()` |
| 08 | volatile + memory-mapped registers | struct overlay, volatile != atomic |
| 09 | reverse list, middle, loop detect, Nth from end | slow/fast pointers (Floyd) |
| 10 | wild / dangling / NULL, return local addr, restrict | set NULL after free |

## Linked lists
(Reverse, middle, loop detection and Nth-from-end are also in PTR-09.)
| # | Topic | Key idea |
|---|-------|----------|
| 01 | singly list CRUD | `node**` walk = no head special case |
| 02 | reverse: iterative, recursive, k-group, m..n | prev/curr/next; recursion costs O(n) stack |
| 03 | merge two / K sorted lists | dummy head node |
| 04 | merge sort a list | split with slow/fast, O(n log n), stable |
| 05 | remove duplicates (sorted / unsorted) | runner O(n^2) or seen-table O(n) |
| 06 | intersection of two lists | switch heads at end; compare addresses |
| 07 | palindrome list | reverse 2nd half, compare, restore |
| 08 | doubly linked list | delete given node in O(1) |
| 09 | circular list, Josephus | keep tail pointer; do-while traversal |
| 10 | add two numbers as lists | carry, unequal lengths |
| 11 | intrusive list (Linux `list_head`) | node inside struct + container_of |
| 12 | LRU cache | hash + doubly linked list, O(1) get/put |

## RTOS
| # | Topic | Key idea |
|---|-------|----------|
| 01 | race condition, mutex | `count++` is load-add-store; mutex has an owner |
| 05 | cooperative time-triggered scheduler | task table + tick; `(now - last) >= period` wraps safely |
| 06 | O(1) priority scheduler | ready bitmap + CLZ, round-robin within a priority |
| 07 | software timers | delta list, ISR touches only the first node |
| 10 | rate monotonic analysis | U <= n(2^(1/n)-1), else response time analysis; EDF U <= 1 |

## Interrupts
| # | Topic | Key idea |
|---|-------|----------|
| 02 | vector table, handler registration | array of fn pointers, weak Default_Handler, VTOR |
| 03 | priorities, nesting, tail-chaining | lower number = higher priority; equal can't preempt |
| 05 | UART RX ISR -> ring buffer -> line parser | never block or print in ISR; count overflows |
| 06 | button debounce | sample every 1 ms, accept after N stable samples |
| 07 | reentrancy | static buffers break it; caller-provided storage |
| 08 | watchdog with task check-in | kick only when ALL tasks report; never from a timer ISR |
| 09 | edge vs level, lost interrupts | one pending bit: drain the FIFO in the ISR; clear level source |

## Drivers
| # | Topic | Key idea |
|---|-------|----------|
| 01 | GPIO driver, register struct | BSRR = atomic set/reset, no read-modify-write race |
| 02 | UART: baud calc, polled TX, IRQ TX | always timeout polling loops; disable TXE IRQ when empty |
| 03 | SPI bit-bang, 4 modes | CPOL = idle level, CPHA = which edge samples |
| 04 | I2C register read/write | S addr+W reg Sr addr+R data NA P; NACK = no device |
| 05 | CRC-8/16/32 bitwise + table | check value of "123456789" |
| 06 | packet parser state machine | byte-at-a-time, resync on SOF, validate len + checksum |
| 07 | driver ops table | Linux file_operations style; mock drivers for tests |
| 09 | ADC scaling, fixed point, filters | multiply before divide; IIR `y += (x - y) >> k` |
| 10 | timer/PWM PSC/ARR, input capture | f = clk/((PSC+1)(ARR+1)); unsigned diff handles wrap |

## Concurrency
Numbered to match the checklist. These files moved here from rtos/, interrupts/ and drivers/.
| # | Topic | Was | Key idea |
|---|-------|-----|----------|
| 01 | spinlock with C11 atomics, CAS | RTOS-11 | acquire on lock, release on unlock; spinning is pointless on one core |
| 02 | counting semaphore from mutex + condvar | RTOS-02 | `while` around cond_wait; wait(empty) before taking the lock |
| 08 | bounded blocking queue | RTOS-08 | mutex + `not_full` / `not_empty`; copy by value; receive timeout |
| 12 | ping-pong DMA buffering | DRV-08 | process one half while DMA fills the other; detect overrun |
| 18 | nestable critical section (+ CON-19 torn read) | INT-04 | save/restore PRIMASK; read hi-lo-hi retry |
| 21 | deferred interrupt processing | INT-01 | short ISR: read HW, clear flag, signal; work in main/task |
| 22 | event flag group | RTOS-09 | wait ANY/ALL; clearing shared bits can hang other waiters |
| 26 | priority inversion / inheritance | RTOS-04 | Mars Pathfinder; mutexes inherit, binary sems don't |
| 27 | deadlock avoidance | RTOS-03 | 4 Coffman conditions; lock ordering, trylock + back off |

Also covered elsewhere: CON-19 in timers/TMR-15, CON-23 = timers/TMR-13 (same problem).

## Timers
Numbered to match the checklist's "Must-code" and "LeetCode-style" items (TMR-13 .. TMR-27).
Concept items TMR-01 .. 12 are explained in the header comments of these files.
| # | Topic | Key idea |
|---|-------|----------|
| 13 | software timer module, v1 + v2 | ISR does `ticks++`; sorted list; unlink before callback; `deadline += period` |
| 14 | `time_after()` with wraparound | `(int32_t)(b - a) < 0`; breaks at 2^31 apart (24.9 days at 1 kHz) |
| 15 | safe 64-bit counter read | read hi, lo, hi again, retry; or count a pending overflow |
| 16 | `delay_ms()` / timeout across wrap | `(now - start) < ms`, never `now < start + ms` |
| 17 | cooperative scheduler, overrun + missed | highest-priority due task; budget check; skip whole missed periods |
| 18 | tickless idle | sleep until earliest deadline, capped by compare range; fix ticks on early wake |
| 19 | debounce: edge IRQ + timer + FSM | mask pin IRQ while debouncing; decide on the timer, not the edges |
| 20 | task watchdog, per-task limits | kick only if every task checked in within its own limit; log culprit |
| 21 | RMS schedulability | utilization bound, response-time analysis, plus hyperperiod simulation |
| 22 | hashed timing wheel | slot = deadline & mask, rounds counter; O(1) insert/cancel |
| 23 | `timer_start()` from ISR | ISR pushes command to SPSC queue; only the timer task touches the list |
| 24 | Task Scheduler (621) | `max(len, (maxf-1)*(n+1) + cnt)` |
| 25 | Meeting Rooms II (253) | sort by start + min-heap of end times |
| 26 | Logger Rate Limiter (359) | hash msg -> next allowed time; fixed table, recycle stale |
| 27 | Hit Counter (362) | 300 buckets of {time, count}; fixed memory |
