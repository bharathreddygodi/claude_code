// Maximum Subarray (Kadane's Algorithm)
// Prompt: Given an integer array nums, find the contiguous subarray (at least
//         one number) with the largest sum, and return that sum.
//
// Idea: for each index, decide: extend the best subarray ending at the
//       previous index, or start fresh here.
//   cur = max(nums[i], cur + nums[i])   // best sum of a subarray ending at i
//   best = max(best, cur)
//   If cur + nums[i] < nums[i], the running sum before i was negative, so it
//   only hurts. Drop it and start over at i.
//   Start cur and best at nums[0] (not 0) so an all-negative array returns
//   its largest element.
// Example: [-2,1,-3,4,-1,2,1,-5,4] -> [4,-1,2,1] = 6.
// Time: O(n). Space: O(1).

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

int maxSubArray(const vector<int>& nums) {
    int cur = nums[0], best = nums[0];
    for (int i = 1; i < (int)nums.size(); i++) {
        cur = max(nums[i], cur + nums[i]);
        best = max(best, cur);
    }
    return best;
}

int main() {
    assert(maxSubArray({-2, 1, -3, 4, -1, 2, 1, -5, 4}) == 6);
    assert(maxSubArray({1}) == 1);
    assert(maxSubArray({5, 4, -1, 7, 8}) == 23);     // whole array
    assert(maxSubArray({-3, -1, -2}) == -1);         // all negative
    assert(maxSubArray({-5}) == -5);
    assert(maxSubArray({2, -10, 3}) == 3);           // restart beats extending

    cout << "All tests passed\n";
    return 0;
}
