// 209. Minimum Size Subarray Sum (LeetCode, Medium)
// Prompt: Given an array of positive integers nums and a positive integer
//         target, return the minimal length of a contiguous subarray whose sum
//         is >= target. If there is none, return 0.
//
// Idea: variable-size sliding window [l, r].
//   - Grow the window by moving r and adding nums[r] to sum.
//   - While sum >= target, the window is valid: record its length, then
//     shrink from the left (subtract nums[l], l++) to look for a shorter one.
//   This works because all numbers are positive: growing always increases the
//   sum and shrinking always decreases it, so each pointer only moves forward.
// Example: target = 7, [2,3,1,2,4,3] -> [4,3] has length 2.
// Follow-up (O(n log n)): prefix sums are increasing, so for each start,
//   binary search for the first prefix >= prefix[start] + target.
// Time: O(n), each index enters and leaves the window once. Space: O(1).

#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <cassert>
using namespace std;

int minSubArrayLen(int target, const vector<int>& nums) {
    int l = 0, sum = 0, best = INT_MAX;
    for (int r = 0; r < (int)nums.size(); r++) {
        sum += nums[r];
        while (sum >= target) {
            best = min(best, r - l + 1);
            sum -= nums[l++];
        }
    }
    return best == INT_MAX ? 0 : best;
}

int main() {
    assert(minSubArrayLen(7, {2, 3, 1, 2, 4, 3}) == 2);     // [4,3]
    assert(minSubArrayLen(4, {1, 4, 4}) == 1);              // single element
    assert(minSubArrayLen(11, {1, 1, 1, 1, 1, 1, 1, 1}) == 0);  // impossible
    assert(minSubArrayLen(15, {1, 2, 3, 4, 5}) == 5);       // whole array
    assert(minSubArrayLen(11, {1, 2, 3, 4, 5}) == 3);       // [3,4,5]
    assert(minSubArrayLen(3, {3}) == 1);
    assert(minSubArrayLen(5, {}) == 0);

    cout << "All tests passed\n";
    return 0;
}
