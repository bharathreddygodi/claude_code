// Min Stack
// Prompt: Design a stack that, besides push and pop, has min() returning the
//         minimum element. push, pop, and min should all be O(1).
//
// Idea: keep a second stack `mins` whose top is always the current minimum.
//   push(x): push x on data; also push on mins if mins is empty or x <= mins top.
//            (<= so duplicate minimums are tracked.)
//   pop():   pop data; if that value == mins top, pop mins too.
//   min():   mins top.
// Time: O(1) for all ops. Space: O(n).

#include <iostream>
#include <cassert>
using namespace std;

const int MAX = 1000;

struct MinStack {
    int data[MAX], mins[MAX];
    int top = -1, minTop = -1;

    void push(int x) {
        if (top == MAX - 1) { cout << "overflow\n"; return; }
        data[++top] = x;
        if (minTop == -1 || x <= mins[minTop]) mins[++minTop] = x;
    }

    int pop() {                      // assumes stack is not empty
        int x = data[top--];
        if (x == mins[minTop]) minTop--;
        return x;
    }

    int peek()     { return data[top]; }
    int getMin()   { return mins[minTop]; }
    bool isEmpty() { return top == -1; }
};

int main() {
    MinStack s;
    s.push(5); assert(s.getMin() == 5);
    s.push(6); assert(s.getMin() == 5);
    s.push(3); assert(s.getMin() == 3);
    s.push(7); assert(s.getMin() == 3);
    assert(s.pop() == 7 && s.getMin() == 3);
    assert(s.pop() == 3 && s.getMin() == 5);

    // duplicate minimums
    s.push(1); s.push(1);
    s.pop();   assert(s.getMin() == 1);
    s.pop();   assert(s.getMin() == 5);

    cout << "All tests passed\n";
    return 0;
}
