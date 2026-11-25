// Remove Duplicates from Sorted List
// Prompt: Given the head of a sorted linked list, delete all duplicates so
//         each element appears only once. Return the (sorted) list.
//
// Idea: the list is sorted, so duplicates are always next to each other.
//   - Walk with cur. If cur->next has the same value, unlink and free it
//     (stay on cur, since the new next might be a duplicate too).
//   - Otherwise move cur forward.
//   The head never changes (the first copy is always kept), so no dummy needed.
// Example: 1->1->2->3->3 -> 1->2->3.
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

ListNode* deleteDuplicates(ListNode* head) {
    ListNode* cur = head;
    while (cur && cur->next) {
        if (cur->next->val == cur->val) {
            ListNode* dup = cur->next;
            cur->next = dup->next;      // unlink the duplicate
            delete dup;
        } else {
            cur = cur->next;
        }
    }
    return head;
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

bool check(const vector<int>& in, const vector<int>& expected) {
    ListNode* res = deleteDuplicates(build(in));
    bool ok = toVector(res) == expected;
    freeList(res);
    return ok;
}

int main() {
    assert(check({1, 1, 2}, {1, 2}));
    assert(check({1, 1, 2, 3, 3}, {1, 2, 3}));
    assert(check({1, 1, 1, 1}, {1}));             // all the same
    assert(check({1, 2, 3}, {1, 2, 3}));          // no duplicates
    assert(check({-2, -2, 0, 5, 5, 5}, {-2, 0, 5}));
    assert(check({7}, {7}));
    assert(check({}, {}));

    cout << "All tests passed\n";
    return 0;
}
