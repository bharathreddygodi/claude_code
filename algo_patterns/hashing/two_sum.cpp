// Two Sum
// Prompt: Given an integer array nums and a target, return the indices of the
//         two numbers that add up to target. Exactly one solution exists, and
//         the same element can't be used twice. Any order is fine.
//
// Idea: one pass with a hash map from value -> index.
//   For each nums[i], the number we need is target - nums[i]. If we've seen
//   it already, we're done. Otherwise remember nums[i] and keep going.
//   Checking before inserting stops an element from pairing with itself.
// Example: [2,7,11,15], target 9 -> at 7, need 2, seen at index 0 -> [0,1].
// Alternative: if the array were sorted, two pointers would do it in O(1) space.
// Time: O(n). Space: O(n).

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>
using namespace std;

vector<int> twoSum(const vector<int>& nums, int target) {
    unordered_map<int, int> seen;              // value -> index
    for (int i = 0; i < (int)nums.size(); i++) {
        int need = target - nums[i];
        if (seen.count(need)) return {seen[need], i};
        seen[nums[i]] = i;
    }
    return {};                                  // no pair (won't happen per prompt)
}

int main() {
    assert(twoSum({2, 7, 11, 15}, 9) == vector<int>({0, 1}));
    assert(twoSum({3, 2, 4}, 6) == vector<int>({1, 2}));     // can't reuse the 3
    assert(twoSum({3, 3}, 6) == vector<int>({0, 1}));        // duplicate values
    assert(twoSum({-3, 4, 3, 90}, 0) == vector<int>({0, 2})); // negatives
    assert(twoSum({1, 2}, 10).empty());

    cout << "All tests passed\n";
    return 0;
}
