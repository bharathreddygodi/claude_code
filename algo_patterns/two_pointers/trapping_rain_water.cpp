// Trapping Rain Water
// Prompt: Given n non-negative integers representing an elevation map (each
//         bar width 1), compute how much water it can trap after raining.
//
// Idea: water above bar i = min(tallest bar to its left, tallest to its right)
//       - height[i]. Two pointers compute this without extra arrays.
//   - l at the left end, r at the right end; lmax/rmax = tallest seen so far
//     from each side.
//   - If height[l] < height[r]: there's a bar on the right at least as tall
//     as height[l], so the left side is the limit. Water at l = lmax - height[l]
//     (after updating lmax). Move l right.
//   - Otherwise do the same from the right side and move r left.
//   We always move the shorter side, because its water level is already known.
// Example: [0,1,0,2,1,0,1,3,2,1,2,1] -> 6.
// Alternatives: prefix/suffix max arrays (O(n) space), or a monotonic stack.
// Time: O(n). Space: O(1).

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

int trap(const vector<int>& height) {
    int l = 0, r = (int)height.size() - 1;
    int lmax = 0, rmax = 0, water = 0;
    while (l < r) {
        if (height[l] < height[r]) {
            lmax = max(lmax, height[l]);
            water += lmax - height[l];
            l++;
        } else {
            rmax = max(rmax, height[r]);
            water += rmax - height[r];
            r--;
        }
    }
    return water;
}

int main() {
    assert(trap({0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1}) == 6);
    assert(trap({4, 2, 0, 3, 2, 5}) == 9);
    assert(trap({3, 0, 3}) == 3);                 // simple bowl
    assert(trap({1, 2, 3, 4}) == 0);              // increasing: nothing held
    assert(trap({4, 3, 2, 1}) == 0);              // decreasing
    assert(trap({5}) == 0);
    assert(trap({}) == 0);

    cout << "All tests passed\n";
    return 0;
}
