// Linked List Cycle
// Prompt: Given the head of a linked list, determine if it has a cycle.
//
// Idea: Floyd's tortoise and hare.
//   - slow moves 1 step, fast moves 2 steps.
//   - No cycle: fast reaches NULL -> false.
//   - Cycle: once both are inside the loop, fast gains 1 step on slow every
//     move, so it must land on slow eventually -> true.
//   Check fast && fast->next before stepping, so fast->next->next is safe.
// The simpler approach is a hash set of visited nodes, but that's O(n) space.
// Time: O(n). Space: O(1).

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(NULL) {}
};

bool hasCycle(ListNode* head) {
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}

// test helper: builds a list of n nodes; tail links back to node `pos` (-1 = no cycle)
// fills `nodes` so the caller can free them even when there's a cycle
ListNode* build(int n, int pos, vector<ListNode*>& nodes) {
    nodes.clear();
    for (int i = 0; i < n; i++) nodes.push_back(new ListNode(i));
    for (int i = 0; i + 1 < n; i++) nodes[i]->next = nodes[i + 1];
    if (n > 0 && pos != -1) nodes[n - 1]->next = nodes[pos];
    return n ? nodes[0] : NULL;
}

int main() {
    vector<ListNode*> nodes;
    struct Case { int n, pos; bool expected; };
    vector<Case> cases = {
        {4, 1, true},     // 0->1->2->3->(back to 1)
        {2, 0, true},     // 0->1->(back to 0)
        {1, 0, true},     // single node pointing to itself
        {1, -1, false},   // single node, no cycle
        {5, -1, false},   // plain list
        {5, 4, true},     // tail points to itself
        {0, -1, false},   // empty list
    };
    for (auto& c : cases) {
        ListNode* head = build(c.n, c.pos, nodes);
        assert(hasCycle(head) == c.expected);
        for (ListNode* p : nodes) delete p;
    }

    cout << "All tests passed\n";
    return 0;
}
