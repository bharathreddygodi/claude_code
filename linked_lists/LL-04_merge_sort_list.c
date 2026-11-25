/*
 * LL-04: Sort a linked list with merge sort - O(n log n), no random access needed
 * Split at the middle with slow/fast pointers, sort halves, merge.
 * Why not quicksort? Needs random access/partitioning; merge sort is stable and
 * natural for lists. Insertion sort is fine for tiny lists (shown too).
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *from_array(const int *a, int n)
{
    node_t *h = NULL, **t = &h;
    int i;
    for (i = 0; i < n; i++) {
        *t = malloc(sizeof **t);
        (*t)->val = a[i]; (*t)->next = NULL;
        t = &(*t)->next;
    }
    return h;
}
static void print(const char *s, const node_t *h) { printf("%-15s:", s); for (; h; h = h->next) printf(" %d", h->val); printf("\n"); }
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }

static node_t *merge(node_t *a, node_t *b)
{
    node_t d = {0, NULL}, *t = &d;
    while (a && b) {
        if (a->val <= b->val) { t->next = a; a = a->next; } else { t->next = b; b = b->next; }
        t = t->next;
    }
    t->next = a ? a : b;
    return d.next;
}

node_t *merge_sort(node_t *h)
{
    node_t *slow, *fast, *right;
    if (!h || !h->next) return h;
    slow = h; fast = h->next;            /* fast starts one ahead -> left half <= right */
    while (fast && fast->next) { slow = slow->next; fast = fast->next->next; }
    right = slow->next;
    slow->next = NULL;                   /* cut */
    return merge(merge_sort(h), merge_sort(right));
}

node_t *insertion_sort(node_t *h)
{
    node_t d = {0, NULL};
    while (h) {
        node_t *nx = h->next, *p = &d;
        while (p->next && p->next->val < h->val) p = p->next;
        h->next = p->next;
        p->next = h;
        h = nx;
    }
    return d.next;
}

int main(void)
{
    int a[] = {38, 27, 43, 3, 9, 82, 10, -5, 27};
    node_t *h = from_array(a, 9);
    print("unsorted", h);
    h = merge_sort(h);
    print("merge_sort", h);
    free_list(h);
    h = insertion_sort(from_array(a, 9));
    print("insertion_sort", h);
    free_list(h);
    return 0;
}
