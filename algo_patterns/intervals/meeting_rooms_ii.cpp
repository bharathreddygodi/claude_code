// Meeting Rooms II
// Prompt: Given an array of meeting time intervals [start, end], find the
//         minimum number of conference rooms required.
//
// Idea: the answer is the maximum number of meetings running at the same time.
//   Sort start times and end times separately, then sweep through the starts
//   in order:
//   - If the next meeting starts before the earliest unfinished meeting ends
//     (start[i] < end[j]), nobody has freed a room -> need a new room.
//   - Otherwise a meeting has ended -> reuse its room (j++), room count stays.
//   We never need to know which meeting ends, only when; that's why
//   sorting the two lists separately is fine.
//   A meeting ending at 5 frees its room for one starting at 5 (strict <).
// Example: [[0,30],[5,10],[15,20]] -> [0,30] overlaps both others -> 2 rooms.
// Alternative: sort by start and keep a min-heap of end times; the heap size
//   at the end is the answer. Same O(n log n).
// Time: O(n log n) for sorting. Space: O(n).

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

int minMeetingRooms(const vector<vector<int>>& intervals) {
    int n = intervals.size();
    vector<int> starts(n), ends(n);
    for (int i = 0; i < n; i++) {
        starts[i] = intervals[i][0];
        ends[i] = intervals[i][1];
    }
    sort(starts.begin(), starts.end());
    sort(ends.begin(), ends.end());

    int rooms = 0, j = 0;
    for (int i = 0; i < n; i++) {
        if (starts[i] < ends[j]) rooms++;   // no room free -> open a new one
        else j++;                           // a meeting ended -> reuse its room
    }
    return rooms;
}

int main() {
    assert(minMeetingRooms({{0, 30}, {5, 10}, {15, 20}}) == 2);
    assert(minMeetingRooms({{7, 10}, {2, 4}}) == 1);              // no overlap
    assert(minMeetingRooms({{1, 5}, {5, 10}}) == 1);              // back-to-back
    assert(minMeetingRooms({{1, 10}, {2, 9}, {3, 8}}) == 3);      // all overlap
    assert(minMeetingRooms({{1, 4}, {2, 5}, {4, 7}, {5, 8}}) == 2);
    assert(minMeetingRooms({}) == 0);

    cout << "All tests passed\n";
    return 0;
}
