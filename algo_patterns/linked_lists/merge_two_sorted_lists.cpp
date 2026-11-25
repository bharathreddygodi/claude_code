// Merge Two Sorted Lists
// Prompt: Given the heads of two sorted linked lists, merge them into one
//         sorted linked list.
//
// Idea: like the merge step of merge sort, using a dummy head.
//   - While both lists have nodes, attach the smaller head to the tail and
//     advance that list.
//   - When one runs out, attach the rest of the other list in one step
//     (it's already sorted).
//   - Reuses the existing nodes (just relinks next pointers), no new nodes.
//   Using <= takes from l1 first on ties, so equal values keep their order.
// Example: 1->2->4 + 1->3->4 -> 1->1->2->3->4->4.
// Time: O(m + n). Space: O(1).

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(NULL) {}
};

ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (l1 && l2) {
        if (l1->val <= l2->val) { 
            tail->next = l1; 
            l1 = l1->next; }
        else                    { 
            tail->next = l2; 
            l2 = l2->next; }
        tail = tail->next;
    }
    tail->next = l1 ? l1 : l2;      // attach whatever is left
    return dummy.next;
}

// ---- test helpers ----
ListNode* build(const vector<int>& vals) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    for (int v : vals) { tail->next = new ListNode(v); tail = tail->next; }
    return dummy.next;
}

vector<int> toVector(ListNode* head) {
    vector<int> v;
    for (; head; head = head->next) v.push_back(head->val);
    return v;
}

void freeList(ListNode* head) {
    while (head) { ListNode* nxt = head->next; delete head; head = nxt; }
}

bool check(const vector<int>& a, const vector<int>& b, const vector<int>& expected) {
    ListNode* merged = mergeTwoLists(build(a), build(b));
    bool ok = toVector(merged) == expected;
    freeList(merged);              // merged owns every node now
    return ok;
}

int main() {
    assert(check({1, 2, 4}, {1, 3, 4}, {1, 1, 2, 3, 4, 4}));
    assert(check({}, {}, {}));
    assert(check({}, {0}, {0}));                         // one list empty
    assert(check({1, 2, 3}, {4, 5, 6}, {1, 2, 3, 4, 5, 6})); // no overlap
    assert(check({5}, {1, 2, 3}, {1, 2, 3, 5}));         // leftover from l1
    assert(check({-3, 0, 7}, {-5, 0, 10}, {-5, -3, 0, 0, 7, 10}));

    cout << "All tests passed\n";
    return 0;
}
