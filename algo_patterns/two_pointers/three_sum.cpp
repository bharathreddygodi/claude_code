// 3Sum
// Prompt: Given an integer array nums, return all unique triplets
//         [nums[i], nums[j], nums[k]] (i, j, k distinct) that sum to 0.
//
// Idea: sort, then fix one number and two-pointer the rest.
//   - Sort nums.
//   - For each i, look for two numbers in nums[i+1..n-1] that sum to -nums[i]:
//     l = i+1, r = n-1. sum < 0 -> l++ (need bigger), sum > 0 -> r-- (need
//     smaller), sum == 0 -> record it and move both.
//   - Skip duplicates: skip i if nums[i] == nums[i-1]; after a match, skip
//     over equal values at l and r.
//   - If nums[i] > 0, stop: everything after it is positive too.
// Example: [-1,0,1,2,-1,-4] -> sorted [-4,-1,-1,0,1,2] -> [[-1,-1,2], [-1,0,1]]
// Time: O(n^2). Space: O(1) extra (ignoring the output and the sort).

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

vector<vector<int>> threeSum(vector<int> nums) {
    vector<vector<int>> res;
    sort(nums.begin(), nums.end());
    int n = nums.size();
    for (int i = 0; i < n - 2; i++) {
        if (nums[i] > 0) break;
        if (i > 0 && nums[i] == nums[i - 1]) continue;   // same first number
        int l = i + 1, r = n - 1;
        while (l < r) {
            int sum = nums[i] + nums[l] + nums[r];
            if (sum < 0) l++;
            else if (sum > 0) r--;
            else {
                res.push_back({nums[i], nums[l], nums[r]});
                l++; r--;
                while (l < r && nums[l] == nums[l - 1]) l++;   // skip dup l
                while (l < r && nums[r] == nums[r + 1]) r--;   // skip dup r
            }
        }
    }
    return res;
}

int main() {
    vector<vector<int>> exp1 = {{-1, -1, 2}, {-1, 0, 1}};
    assert(threeSum({-1, 0, 1, 2, -1, -4}) == exp1);

    vector<vector<int>> exp2 = {{0, 0, 0}};
    assert(threeSum({0, 0, 0, 0}) == exp2);               // duplicates collapse

    assert(threeSum({0, 1, 1}).empty());                  // no answer
    assert(threeSum({1, 2}).empty());                     // fewer than 3

    vector<vector<int>> exp3 = {{-2, -2, 4}, {-2, 0, 2}};
    assert(threeSum({-2, -2, -2, 0, 2, 2, 4}) == exp3);   // many dups

    cout << "All tests passed\n";
    return 0;
}
