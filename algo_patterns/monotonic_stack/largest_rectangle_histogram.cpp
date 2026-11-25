// Largest Rectangle in Histogram
// Prompt: Given an array of bar heights (each bar width 1), find the area of
//         the largest rectangle in the histogram.
//
// Idea: monotonic increasing stack of indices.
//   For each bar, the best rectangle using that bar's full height stretches
//   left and right until it hits a shorter bar. The stack finds both edges:
//   - Keep indices with increasing heights.
//   - When bar i is shorter than the top bar, the top bar can't extend past i,
//     so pop it. Its right edge is i, its left edge is the new stack top
//     (the previous shorter bar). width = i - newTop - 1, or i if stack empty.
//   - Run one extra step with height 0 at i = n to flush the stack.
// Example: [2,1,5,6,2,3] -> bars 5 and 6 give height 5 * width 2 = 10.
// Time: O(n), each index pushed and popped once. Space: O(n).

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cassert>
using namespace std;

int largestRectangleArea(vector<int>& heights) {
    int n = heights.size(), best = 0;
    stack<int> st;                                   // indices, heights increasing
    for (int i = 0; i <= n; i++) {
        int h = (i == n) ? 0 : heights[i];           // 0 at the end flushes the stack
        while (!st.empty() && heights[st.top()] >= h) {
            int height = heights[st.top()]; st.pop();
            int width = st.empty() ? i : i - st.top() - 1;
            best = max(best, height * width);
        }
        st.push(i);
    }
    return best;
}

int main() {
    vector<int> a = {2, 1, 5, 6, 2, 3};
    assert(largestRectangleArea(a) == 10);
    vector<int> b = {2, 4};
    assert(largestRectangleArea(b) == 4);
    vector<int> c = {1, 2, 3, 4, 5};      // increasing: 3 * 3 = 9
    assert(largestRectangleArea(c) == 9);
    vector<int> d = {5, 4, 3, 2, 1};      // decreasing: 3 * 3 = 9
    assert(largestRectangleArea(d) == 9);
    vector<int> e = {3, 3, 3};            // all equal: 3 * 3
    assert(largestRectangleArea(e) == 9);
    vector<int> f = {2, 1, 2};            // shortest bar spans all: 1 * 3
    assert(largestRectangleArea(f) == 3);
    vector<int> g = {0};
    assert(largestRectangleArea(g) == 0);
    vector<int> h = {};
    assert(largestRectangleArea(h) == 0);

    cout << "All tests passed\n";
    return 0;
}
