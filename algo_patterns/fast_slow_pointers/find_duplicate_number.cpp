// Find the Duplicate Number
// Prompt: nums has n + 1 integers, each in [1, n]. Exactly one value is
//         repeated (possibly more than twice). Return it without modifying
//         nums and using only O(1) extra space.
//
// Idea: treat the array as a linked list and use Floyd's cycle detection.
//   - Think of each index i as a node with an edge i -> nums[i].
//   - Values are in [1, n], so following edges from index 0 never leaves the
//     array, and index 0 is never pointed back to.
//   - The duplicate value d is pointed to by two (or more) indices, so the
//     path from 0 runs into a cycle, and d is where the cycle starts.
//   Phase 1: slow moves 1 step, fast moves 2 steps, until they meet in the cycle.
//   Phase 2: reset slow to 0, move both 1 step at a time. They meet at the
//            cycle start = the duplicate.
//   (Why phase 2 works: if the cycle starts after a steps from 0, the meeting
//    point is also a steps, mod the cycle length, from the cycle start.)
// Example: [1,3,4,2,2] -> 0->1->3->2->4->2->4... cycle starts at 2 -> answer 2.
// Time: O(n). Space: O(1). The array is only read.

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

int findDuplicate(const vector<int>& nums) {
    int slow = nums[0], fast = nums[nums[0]];
    while (slow != fast) {               // phase 1: find a meeting point
        slow = nums[slow];
        fast = nums[nums[fast]];
    }
    slow = 0;
    while (slow != fast) {               // phase 2: find the cycle start
        slow = nums[slow];
        fast = nums[fast];
    }
    return slow;
}

int main() {
    assert(findDuplicate({1, 3, 4, 2, 2}) == 2);
    assert(findDuplicate({3, 1, 3, 4, 2}) == 3);
    assert(findDuplicate({3, 3, 3, 3, 3}) == 3);     // repeated many times
    assert(findDuplicate({1, 1}) == 1);              // smallest case, n = 1
    assert(findDuplicate({2, 5, 9, 6, 9, 3, 8, 9, 7, 1}) == 9);

    cout << "All tests passed\n";
    return 0;
}
