/*
 * PTR-09: Linked-list pointer manipulation (asked in almost every C interview)
 *  - reverse a list iteratively (prev/curr/next)
 *  - find the middle (slow/fast pointers)
 *  - detect a loop and find its start (Floyd's tortoise & hare)
 *  - Nth node from the end (two pointers N apart)
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *build(int n)
{
    node_t *head = NULL, **tail = &head;
    int i;
    for (i = 1; i <= n; i++) {
        *tail = malloc(sizeof **tail);
        (*tail)->val = i;
        (*tail)->next = NULL;
        tail = &(*tail)->next;
    }
    return head;
}

static void print(const node_t *n) { for (; n; n = n->next) printf("%d ", n->val); printf("\n"); }

node_t *reverse(node_t *head)
{
    node_t *prev = NULL;
    while (head) {
        node_t *next = head->next;
        head->next = prev;
        prev = head;
        head = next;
    }
    return prev;
}

node_t *middle(node_t *h)
{
    node_t *slow = h, *fast = h;
    while (fast && fast->next) { slow = slow->next; fast = fast->next->next; }
    return slow;
}

node_t *loop_start(node_t *h)        /* NULL if no loop */
{
    node_t *slow = h, *fast = h;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {          /* met inside loop; restart one from head */
            slow = h;
            while (slow != fast) { slow = slow->next; fast = fast->next; }
            return slow;
        }
    }
    return NULL;
}

node_t *nth_from_end(node_t *h, int n)
{
    node_t *lead = h, *trail = h;
    while (n-- > 0) { if (!lead) return NULL; lead = lead->next; }
    while (lead) { lead = lead->next; trail = trail->next; }
    return trail;
}

static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }

int main(void)
{
    node_t *h = build(7), *t, *start;

    printf("list      : "); print(h);
    printf("middle    : %d\n", middle(h)->val);
    printf("2nd from end: %d\n", nth_from_end(h, 2)->val);
    h = reverse(h);
    printf("reversed  : "); print(h);
    printf("loop?     : %s\n", loop_start(h) ? "yes" : "no");

    for (t = h; t->next; t = t->next) { }
    t->next = h->next->next;          /* tail -> 3rd node (value 5) makes a loop */
    start = loop_start(h);
    printf("loop start: %d (expect 5)\n", start ? start->val : -1);
    t->next = NULL;                   /* break loop before freeing */
    free_list(h);
    return 0;
}
