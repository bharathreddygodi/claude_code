// Copy List with Random Pointer
// Prompt: Each node has next and an extra random pointer (to any node or
//         NULL). Return a deep copy of the list.
// Challenge: O(1) extra space (no hash map from old node -> new node).
//
// Idea: weave the copies into the original list, so each copy sits right
//       after its original. Then "the copy of X" is just X->next.
//   1. Weave:  A -> B -> C   becomes   A -> A' -> B -> B' -> C -> C'
//   2. Random: A'->random = A->random ? A->random->next : NULL
//              (A->random->next is the copy of whatever A pointed to.)
//   3. Unweave: split into the original list and the copy list, putting the
//      original's next pointers back the way they were.
// The simpler approach is a hash map old -> new (two passes), but that's O(n)
// extra space.
// Time: O(n), three passes. Space: O(1) extra (the copies are the output).

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

struct Node {
    int val;
    Node* next;
    Node* random;
    Node(int v) : val(v), next(NULL), random(NULL) {}
};

Node* copyRandomList(Node* head) {
    if (!head) return NULL;

    // 1. weave a copy after each node
    for (Node* cur = head; cur; cur = cur->next->next) {
        Node* copy = new Node(cur->val);
        copy->next = cur->next;
        cur->next = copy;
    }

    // 2. set random pointers on the copies
    for (Node* cur = head; cur; cur = cur->next->next) {
        cur->next->random = cur->random ? cur->random->next : NULL;
    }

    // 3. unweave: restore the original, build the copy list
    Node* newHead = head->next;
    for (Node* cur = head; cur; cur = cur->next) {
        Node* copy = cur->next;
        cur->next = copy->next;                         // restore original
        copy->next = copy->next ? copy->next->next : NULL;
    }
    return newHead;
}

// ---- test helpers ----

// vals[i] is the value of node i; rand[i] is the index node i's random points to (-1 = NULL)
Node* build(const vector<int>& vals, const vector<int>& rand) {
    int n = vals.size();
    if (n == 0) return NULL;
    vector<Node*> nodes(n);
    for (int i = 0; i < n; i++) nodes[i] = new Node(vals[i]);
    for (int i = 0; i < n; i++) {
        if (i + 1 < n) nodes[i]->next = nodes[i + 1];
        if (rand[i] != -1) nodes[i]->random = nodes[rand[i]];
    }
    return nodes[0];
}

int indexOf(Node* head, Node* target) {
    if (!target) return -1;
    int i = 0;
    for (Node* p = head; p; p = p->next, i++) if (p == target) return i;
    return -2;   // not in this list
}

// copy must have the same values and random structure, with no shared nodes
bool isDeepCopy(Node* orig, Node* copy) {
    for (Node *a = orig, *b = copy; a || b; a = a->next, b = b->next) {
        if (!a || !b || a == b || a->val != b->val) return false;
        if (indexOf(orig, a->random) != indexOf(copy, b->random)) return false;
        if (b->random && indexOf(orig, b->random) != -2) return false;  // points into original
    }
    return true;
}

bool matches(Node* head, const vector<int>& vals, const vector<int>& rand) {
    int i = 0;
    for (Node* p = head; p; p = p->next, i++) {
        if (i >= (int)vals.size() || p->val != vals[i]) return false;
        if (indexOf(head, p->random) != rand[i]) return false;
    }
    return i == (int)vals.size();
}

void freeList(Node* head) {
    while (head) { Node* nxt = head->next; delete head; head = nxt; }
}

int main() {
    vector<vector<int>> vals  = {{7, 13, 11, 10, 1}, {1, 2}, {3, 3, 3}, {5}, {}};
    vector<vector<int>> rands = {{-1, 0, 4, 2, 0},   {1, 1}, {-1, 0, -1}, {0}, {}};

    for (int t = 0; t < (int)vals.size(); t++) {
        Node* orig = build(vals[t], rands[t]);
        Node* copy = copyRandomList(orig);
        assert(isDeepCopy(orig, copy));
        assert(matches(orig, vals[t], rands[t]));   // original left unchanged
        assert(matches(copy, vals[t], rands[t]));
        freeList(orig);
        freeList(copy);
    }

    cout << "All tests passed\n";
    return 0;
}
