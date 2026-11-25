// Binary Search
// Prompt: Given a sorted (ascending, distinct) array nums and a target, return
//         the index of target, or -1 if it's not there. Must be O(log n).
//
// Idea: keep a window [lo, hi] that must contain target if it exists.
//   - mid = lo + (hi - lo) / 2   (avoids overflow of lo + hi)
//   - nums[mid] == target -> found.
//   - nums[mid] <  target -> target is to the right: lo = mid + 1.
//   - nums[mid] >  target -> target is to the left:  hi = mid - 1.
//   Each step halves the window; stop when lo > hi (window empty) -> -1.
// Example: nums = [-1,0,3,5,9,12], target = 9 -> mid 2 (3) -> mid 4 (9) -> 4.
// Time: O(log n). Space: O(1).

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

int search(const vector<int>& nums, int target) {
    int lo = 0, hi = (int)nums.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (nums[mid] == target) return mid;
        if (nums[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int main() {
    vector<int> a = {-1, 0, 3, 5, 9, 12};
    assert(search(a, 9) == 4);
    assert(search(a, 2) == -1);       // in range but missing
    assert(search(a, -1) == 0);       // first element
    assert(search(a, 12) == 5);       // last element
    assert(search(a, -5) == -1);      // smaller than everything
    assert(search(a, 100) == -1);     // larger than everything

    assert(search({5}, 5) == 0);
    assert(search({5}, 3) == -1);
    assert(search({}, 1) == -1);

    cout << "All tests passed\n";
    return 0;
}
