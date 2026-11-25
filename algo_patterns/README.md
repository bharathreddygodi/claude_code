# algo_patterns

Interview-style algorithm problems solved in C++. Each problem is a single
self-contained `.cpp` file: the prompt and approach in a short header comment,
the solution, and a small `main()` with a few `assert` checks.

Style: written the way you'd write it on a whiteboard in an interview —
`using namespace std;`, plain `int`s, no templates. Hand-built data
structures are C-style (fixed arrays + an index) rather than wrapping STL.

## Layout

Problems are grouped by category folder. New categories get added as needed
(e.g. `arrays_and_strings/`, `linked_lists/`, `trees_and_graphs/`,
`dynamic_programming/`, `sorting_and_searching/`, `bit_manipulation/`).

```
algo_patterns/
├── README.md
├── binary_search/
│   └── binary_search.cpp
├── bit_manipulation/
│   └── block_compression_2x2.cpp
├── dynamic_programming/
│   ├── decode_ways.cpp
│   └── max_subarray.cpp
├── fast_slow_pointers/
│   ├── find_duplicate_number.cpp
│   └── linked_list_cycle.cpp
├── hashing/
│   └── two_sum.cpp
├── intervals/
│   └── meeting_rooms_ii.cpp
├── linked_lists/
│   ├── add_two_numbers.cpp
│   ├── copy_list_random_pointer.cpp
│   ├── merge_two_sorted_lists.cpp
│   └── remove_duplicates_sorted_list.cpp
├── monotonic_stack/
│   ├── largest_rectangle_histogram.cpp
│   └── remove_k_digits.cpp
├── prefix_sum/
│   ├── continuous_subarray_sum.cpp
│   └── smallest_stable_index_i.cpp
├── sliding_window/
│   └── min_size_subarray_sum.cpp
├── stacks_and_queues/
│   ├── min_stack.cpp
│   └── sort_stack.cpp
└── two_pointers/
    ├── container_with_most_water.cpp
    ├── three_sum.cpp
    └── trapping_rain_water.cpp
```

## Build & run a problem

```bash
g++ -std=c++17 -Wall -Wextra -O2 stacks_and_queues/min_stack.cpp -o min_stack && ./min_stack
```

## Index

| Category | Problem | Pattern | Complexity |
|---|---|---|---|
| Stacks & Queues | [Min Stack](stacks_and_queues/min_stack.cpp) | Auxiliary shadow stack | O(1) push/pop/min, O(n) space |
| Stacks & Queues | [Sort Stack](stacks_and_queues/sort_stack.cpp) | Insertion sort with a temp stack | O(n²) time, O(n) space |
| Monotonic Stack | [Remove K Digits](monotonic_stack/remove_k_digits.cpp) | Greedy + increasing stack | O(n) time, O(n) space |
| Monotonic Stack | [Largest Rectangle in Histogram](monotonic_stack/largest_rectangle_histogram.cpp) | Increasing stack of indices, find nearest smaller on both sides | O(n) time, O(n) space |
| Two Pointers | [3Sum](two_pointers/three_sum.cpp) | Sort + fix one, two-pointer the rest | O(n²) time, O(1) extra space |
| Two Pointers | [Trapping Rain Water](two_pointers/trapping_rain_water.cpp) | Move the shorter side, track left/right max | O(n) time, O(1) space |
| Two Pointers | [Container With Most Water](two_pointers/container_with_most_water.cpp) | Start widest, move the shorter line inward | O(n) time, O(1) space |
| Fast & Slow Pointers | [Find the Duplicate Number](fast_slow_pointers/find_duplicate_number.cpp) | Array as linked list + Floyd's cycle detection | O(n) time, O(1) space |
| Fast & Slow Pointers | [Linked List Cycle](fast_slow_pointers/linked_list_cycle.cpp) | Floyd's tortoise and hare | O(n) time, O(1) space |
| Dynamic Programming | [Maximum Subarray](dynamic_programming/max_subarray.cpp) | Kadane's: extend or restart at each index | O(n) time, O(1) space |
| Dynamic Programming | [Decode Ways](dynamic_programming/decode_ways.cpp) | Last letter used 1 or 2 digits, keep last two counts | O(n) time, O(1) space |
| Intervals | [Meeting Rooms II](intervals/meeting_rooms_ii.cpp) | Sort starts and ends separately, sweep | O(n log n) time, O(n) space |
| Binary Search | [Binary Search](binary_search/binary_search.cpp) | Halve the [lo, hi] window each step | O(log n) time, O(1) space |
| Bit Manipulation | [2×2 Block Image Compression](bit_manipulation/block_compression_2x2.cpp) | Block average + four 2-bit codes packed in a byte | O(W·H) time, O(1) extra space |
| Hashing | [Two Sum](hashing/two_sum.cpp) | One pass, hash map of value → index | O(n) time, O(n) space |
| Linked Lists | [Add Two Numbers](linked_lists/add_two_numbers.cpp) | Column addition with carry, dummy head | O(max(m, n)) time, O(1) extra space |
| Linked Lists | [Copy List with Random Pointer](linked_lists/copy_list_random_pointer.cpp) | Weave copies in, set random, unweave | O(n) time, O(1) extra space |
| Linked Lists | [Merge Two Sorted Lists](linked_lists/merge_two_sorted_lists.cpp) | Merge step with a dummy head, relink nodes | O(m + n) time, O(1) space |
| Linked Lists | [Remove Duplicates from Sorted List](linked_lists/remove_duplicates_sorted_list.cpp) | Duplicates are adjacent, unlink cur->next while equal | O(n) time, O(1) space |
| Prefix Sum | [Continuous Subarray Sum](prefix_sum/continuous_subarray_sum.cpp) | Equal prefix remainders mod k, hash map of first index | O(n) time, O(min(n, k)) space |
| Prefix Sum | [3903. Smallest Stable Index I](prefix_sum/smallest_stable_index_i.cpp) | Suffix-min array + running prefix max | O(n) time, O(n) space |
| Sliding Window | [209. Minimum Size Subarray Sum](sliding_window/min_size_subarray_sum.cpp) | Grow right, shrink left while sum >= target | O(n) time, O(1) space |
