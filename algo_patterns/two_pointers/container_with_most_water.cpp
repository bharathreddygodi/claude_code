// Container With Most Water
// Prompt: height[i] is a vertical line at index i. Pick two lines; their area
//         is (j - i) * min(height[i], height[j]). Return the largest area
//         without changing the input. Fewer than two lines -> 0.
//         n <= 100000, heights <= 10000, so the max area (~1e9) fits in int.
//
// Idea: two pointers, starting at the widest pair (both ends).
//   - Area of the current pair = (r - l) * min(height[l], height[r]).
//   - Move the pointer at the shorter line inward.
//     Why: every other pair that uses the shorter line is narrower and still
//     capped by that same short height, so none can beat the current area.
//     That line can be dropped. Moving the taller one could never help.
//   - Stop when the pointers meet.
// Example: [1,8,6,2,5,4,8,3,7] -> lines at index 1 (8) and 8 (7):
//          7 * min(8,7) = 49.
// Time: O(n). Space: O(1).

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

int maxArea(const vector<int>& height) {
    int l = 0, r = (int)height.size() - 1, best = 0;
    while (l < r) {
        int area = (r - l) * min(height[l], height[r]);
        best = max(best, area);
        if (height[l] < height[r]) l++;
        else r--;
    }
    return best;
}

int main() {
    assert(maxArea({1, 8, 6, 2, 5, 4, 8, 3, 7}) == 49);
    assert(maxArea({1, 1}) == 1);
    assert(maxArea({4, 3, 2, 1, 4}) == 16);      // the two ends win
    assert(maxArea({1, 2, 1}) == 2);
    assert(maxArea({0, 0, 0}) == 0);             // all zero heights
    assert(maxArea({5}) == 0);                   // fewer than two lines
    assert(maxArea({}) == 0);

    // largest allowed input: 100000 lines of height 10000
    vector<int> big(100000, 10000);
    assert(maxArea(big) == 99999 * 10000);       // 999,990,000 fits in int

    cout << "All tests passed\n";
    return 0;
}
