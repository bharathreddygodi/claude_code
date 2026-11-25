// 3903. Smallest Stable Index I (LeetCode, Easy)
// Prompt: Given nums (length n) and k. The instability score of index i is
//         max(nums[0..i]) - min(nums[i..n-1]). Index i is stable if its score
//         is <= k. Return the smallest stable index, or -1 if there is none.
//         1 <= n <= 100, 0 <= nums[i], k <= 1e9.
//
// Idea: precompute suffix minimums, then scan left to right with a running max.
//   - sufMin[i] = min(nums[i], sufMin[i+1]), built right to left.
//   - Walk i = 0..n-1 keeping preMax = max(nums[0..i]).
//   - The first i with preMax - sufMin[i] <= k is the answer.
//   The score is never negative: preMax >= nums[i] >= sufMin[i].
//   It also fits in int: both values are in [0, 1e9].
// Example: [5,0,1,4], k = 3 -> scores 5, 5, 4, 1 -> first <= 3 is index 3.
// Time: O(n). Space: O(n) for the suffix-min array.

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

int smallestStableIndex(const vector<int>& nums, int k) {
    int n = nums.size();
    vector<int> sufMin(n);
    sufMin[n - 1] = nums[n - 1];
    for (int i = n - 2; i >= 0; i--) sufMin[i] = min(nums[i], sufMin[i + 1]);

    int preMax = nums[0];
    for (int i = 0; i < n; i++) {
        preMax = max(preMax, nums[i]);
        if (preMax - sufMin[i] <= k) return i;
    }
    return -1;
}

int main() {
    assert(smallestStableIndex({5, 0, 1, 4}, 3) == 3);
    assert(smallestStableIndex({3, 2, 1}, 1) == -1);
    assert(smallestStableIndex({0}, 0) == 0);
    assert(smallestStableIndex({8, 2, 5, 7}, 3) == 2);       // scores 6, 6, 3, 1
    assert(smallestStableIndex({1, 2, 3}, 0) == 0);          // sorted: score 0 at index 0
    assert(smallestStableIndex({1000000000, 0}, 999999999) == -1);  // large values, scores 1e9 and 1e9
    assert(smallestStableIndex({1000000000, 0}, 1000000000) == 0);

    cout << "All tests passed\n";
    return 0;
}
