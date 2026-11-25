# Memory - C Interview Solutions

Numbered to match "Google Embedded Interview - Coding Study Checklist.md" (MEM-01 .. MEM-16).
Files without a MEM number are extra topics that aren't on that list.

Build and run all: `sh build_and_run.sh`   |   Run one: `sh build_and_run.sh MEM-06`
(or from the parent folder: `sh run.sh memory`)
Each file is self-contained, has its own `main()` with PASS/FAIL checks, and a header
comment listing the talking points interviewers usually look for.

| #  | File | Topic | Key idea to say out loud |
|----|------|-------|--------------------------|
| 01 | MEM-01_aligned_malloc.c | aligned malloc/free | over-allocate, round up, stash raw ptr at `[-1]` |
| 02 | MEM-02_memmove.c | memmove | copy backward when `dst > src` |
| 03 | MEM-03_memcpy.c | optimized memcpy | byte head until aligned, 4x unrolled word copy, leftover words, byte tail |
| 04 | MEM-04_memset_memcmp.c | memset + memcmp | word pattern fill; compare as unsigned, words then bytes |
| 05 | MEM-05_fixed_block_pool.c | fixed-block pool | free list stored inside free blocks, O(1), deterministic |
| 06 | MEM-06_simple_malloc_free.c | own malloc/free | header, first-fit vs best-fit, split, coalesce, double-free check |
| 07 | MEM-07_ring_buffer.c | SPSC ring buffer | power-of-2 mask, free-running head/tail, ISR-safe |
| 08 | MEM-08_memory_layout_stack_direction.c | stack direction + layout | compare locals in caller/callee, `noinline`; also MEM-17 |
| 10 | MEM-10_buddy_allocator.c | buddy allocator | split power-of-2 blocks; buddy = `off ^ (1 << k)`; merge on free |
| 11 | MEM-11_byte_swap.c | endian-safe read from bytes | `read_be32` builds the value with shifts, no pointer cast |
| 12 | MEM-12_dynamic_2d_array.c | 2D array, single malloc | pointer table + data in one block, one `free` |
| 13 | MEM-13_realloc_calloc.c | realloc + calloc | `realloc(NULL)`, `realloc(p,0)`, keep old on failure, overflow check |
| 14 | MEM-14_guard_bytes_canaries.c | guard bytes / canaries | front + back canary, magic for double free, poison freed memory |
| 15 | MEM-15_struct_padding_packing.c | padding / packing | order members large -> small, `pack(1)` cost, bit-fields |
| 16 | MEM-16_pointer_puzzles.c | pointer puzzles | `int (*p)[10]` vs `int *p[10]`; no `void *` math in std C; pun with memcpy |

Extra topics (not in MEM-01 .. MEM-16):
| File | Topic | Key idea |
|------|-------|----------|
| endianness_check.c | detect endianness at runtime | pointer cast / union / `__BYTE_ORDER__` (checklist BIT-17) |
| leak_detector.c | leak / double-free detector | `__FILE__/__LINE__` wrappers, `SAFE_FREE` (checklist MEM-23) |
| offsetof_container_of.c | offsetof / container_of | `(char*)ptr - offsetof`, intrusive lists |

MEM-09 bitmap allocator is solved in bits/BIT-22 (same problem, one file).
MEM-01 to MEM-16 are all covered.

Quick-fire facts:
- `malloc` returns memory aligned for any type; `free(NULL)` is a no-op.
- Dangling pointer = pointer to freed memory; set to NULL after free.
- Stack overflow causes: deep recursion, big local arrays. Heap issue: fragmentation.
- `volatile` = don't cache/optimise away (HW registers, ISR-shared vars); it is NOT atomic.
- Tools: Valgrind, `-fsanitize=address`, stack canaries / watermarking the stack.
