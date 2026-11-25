// Sort Stack
// Prompt: Sort a stack so the smallest items are on top. You may use one extra
//         temporary stack, but no other data structure (e.g. an array).
//
// Idea: like insertion sort, using the temp stack `r` as the sorted part
//       (largest on top).
//   1. Pop tmp from s.
//   2. While r's top > tmp, move r's top back onto s (makes room for tmp).
//   3. Push tmp onto r. Now r is still sorted.
//   Repeat until s is empty, then pour r back into s: the largest goes in
//   first, so the smallest ends up on top.
// Time: O(n^2). Space: O(n) for the temp stack.

#include <iostream>
#include <stack>
#include <cassert>
using namespace std;

void sortStack(stack<int>& s) {
    stack<int> r;
    while (!s.empty()) {
        int tmp = s.top(); s.pop();
        while (!r.empty() && r.top() > tmp) {
            s.push(r.top()); r.pop();
        }
        r.push(tmp);
    }
    while (!r.empty()) {
        s.push(r.top()); r.pop();
    }
}

// helper for tests: pops everything, checks it comes out in ascending order
bool popsAscending(stack<int>& s) {
    while (!s.empty()) {
        int x = s.top(); s.pop();
        if (!s.empty() && x > s.top()) return false;
    }
    return true;
}

int main() {
    stack<int> s;
    for (int x : {5, 1, 4, 2, 8, 3}) s.push(x);
    sortStack(s);
    assert(s.top() == 1 && s.size() == 6);
    assert(popsAscending(s));

    // duplicates
    for (int x : {3, 1, 3, 2, 1}) s.push(x);
    sortStack(s);
    assert(s.top() == 1 && popsAscending(s));

    // already sorted and reverse sorted
    for (int x : {5, 4, 3, 2, 1}) s.push(x);   // 1 on top already
    sortStack(s); assert(popsAscending(s));
    for (int x : {1, 2, 3, 4, 5}) s.push(x);   // 5 on top
    sortStack(s); assert(s.top() == 1 && popsAscending(s));

    // empty and single element
    sortStack(s); assert(s.empty());
    s.push(7); sortStack(s); assert(s.top() == 7 && s.size() == 1);

    cout << "All tests passed\n";
    return 0;
}
