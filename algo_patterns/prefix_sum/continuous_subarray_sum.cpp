// Continuous Subarray Sum
// Prompt: Given an integer array nums and an integer k, return true if there's
//         a contiguous subarray of size at least 2 whose sum is a multiple of k.
//         (Assumes k >= 1, as in the standard version.)
//
// Idea: prefix sums + remainders in a hash map.
//   - sum(i+1..j) = prefix[j] - prefix[i]. That's a multiple of k exactly when
//     prefix[j] % k == prefix[i] % k.
//   - So walk the array keeping the running remainder, and store the FIRST
//     index where each remainder appeared. If the same remainder shows up
//     again at least 2 indices later, the elements between them work.
//   - Seed the map with remainder 0 at index -1 so a prefix that's already a
//     multiple of k (starting at index 0) counts.
//   - Keep only the remainder, not the full sum, so it can't overflow.
//     ((x % k) + k) % k keeps it non-negative if nums has negatives.
// Example: [23,2,4,6,7], k = 6 -> remainders 5,1,5 -> 5 repeats at
//          indices 0 and 2 -> [2,4] sums to 6 -> true.
// Time: O(n). Space: O(min(n, k)).

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>
using namespace std;

bool checkSubarraySum(const vector<int>& nums, int k) {
    unordered_map<int, int> firstIdx;          // remainder -> first index seen
    firstIdx[0] = -1;
    int rem = 0;
    for (int i = 0; i < (int)nums.size(); i++) {
        rem = ((rem + nums[i]) % k + k) % k;
        if (firstIdx.count(rem)) {
            if (i - firstIdx[rem] >= 2) return true;
        } else {
            firstIdx[rem] = i;                 // keep only the earliest index
        }
    }
    return false;
}

int main() {
    assert(checkSubarraySum({23, 2, 4, 6, 7}, 6) == true);    // [2,4]
    assert(checkSubarraySum({23, 2, 6, 4, 7}, 6) == true);    // whole array = 42
    assert(checkSubarraySum({23, 2, 6, 4, 7}, 13) == false);
    assert(checkSubarraySum({5, 0, 0, 0}, 3) == true);        // [0,0] sums to 0
    assert(checkSubarraySum({0}, 1) == false);                // size must be >= 2
    assert(checkSubarraySum({6}, 6) == false);                // single element
    assert(checkSubarraySum({1, 0}, 2) == false);             // sum 1
    assert(checkSubarraySum({2, 4}, 6) == true);              // prefix from index 0
    assert(checkSubarraySum({-1, 7}, 6) == true);             // negatives: -1 + 7 = 6

    cout << "All tests passed\n";
    return 0;
}
