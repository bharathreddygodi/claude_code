// Add Two Numbers (reverse order)
// Prompt: Two numbers are stored as linked lists, one digit per node, in
//         reverse order (1's digit at the head). Add them and return the sum
//         as a linked list, also in reverse order.
//
// Idea: grade-school addition. The heads are the 1's digits, so walk both
//       lists together, just like adding column by column from the right.
//   - digit = (a + b + carry) % 10, carry = (a + b + carry) / 10.
//   - If one list runs out, treat its digit as 0.
//   - Keep going while either list has nodes OR carry is left
//     (e.g. 5 + 5 = 10 needs an extra node for the final 1).
//   - A dummy head node avoids special-casing the first node.
// Example: 342 + 465 = 807 -> (2->4->3) + (5->6->4) = 7->0->8.
// Time: O(max(m, n)). Space: O(1) extra (the result list is the output).

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(NULL) {}
};

ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    int carry = 0;
    while (l1 || l2 || carry) {
        int sum = carry;
        if (l1) { sum += l1->val; l1 = l1->next; }
        if (l2) { sum += l2->val; l2 = l2->next; }
        carry = sum / 10;
        tail->next = new ListNode(sum % 10);
        tail = tail->next;
    }
    return dummy.next;
}

// ---- test helpers ----
ListNode* build(const vector<int>& digits) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    for (int d : digits) { tail->next = new ListNode(d); tail = tail->next; }
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
    ListNode *l1 = build(a), *l2 = build(b);
    ListNode* res = addTwoNumbers(l1, l2);
    bool ok = toVector(res) == expected;
    freeList(l1); freeList(l2); freeList(res);
    return ok;
}

int main() {
    assert(check({2, 4, 3}, {5, 6, 4}, {7, 0, 8}));            // 342 + 465 = 807
    assert(check({0}, {0}, {0}));
    assert(check({9, 9, 9, 9}, {9, 9}, {8, 9, 0, 0, 1}));      // 9999 + 99 = 10098
    assert(check({5}, {5}, {0, 1}));                           // final carry -> new node
    assert(check({1, 8}, {0}, {1, 8}));                        // different lengths
    assert(check({}, {7, 3}, {7, 3}));                         // one list empty

    cout << "All tests passed\n";
    return 0;
}
