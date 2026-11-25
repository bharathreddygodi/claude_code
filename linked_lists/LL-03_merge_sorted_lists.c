/*
 * LL-03: Merge two sorted lists (in place, no new nodes)
 * Dummy-head trick removes the special case for the first node. O(n+m), O(1).
 * Also: merge K sorted lists by repeated pairwise merging (O(N log K)).
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
static void print(const char *s, const node_t *h) { printf("%-10s:", s); for (; h; h = h->next) printf(" %d", h->val); printf("\n"); }
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }

node_t *merge(node_t *a, node_t *b)
{
    node_t dummy = {0, NULL}, *t = &dummy;
    while (a && b) {
        if (a->val <= b->val) { t->next = a; a = a->next; }   /* <= keeps it stable */
        else                  { t->next = b; b = b->next; }
        t = t->next;
    }
    t->next = a ? a : b;
    return dummy.next;
}

node_t *merge_k(node_t **lists, int k)
{
    int step, i;
    if (k == 0) return NULL;
    for (step = 1; step < k; step *= 2)
        for (i = 0; i + step < k; i += 2 * step)
            lists[i] = merge(lists[i], lists[i + step]);
    return lists[0];
}

int main(void)
{
    int a[] = {1, 4, 7, 10}, b[] = {2, 3, 8}, c[] = {0, 5, 6, 11, 12};
    node_t *m = merge(from_array(a, 4), from_array(b, 3));
    print("merge 2", m);
    free_list(m);

    node_t *ls[3] = {from_array(a, 4), from_array(b, 3), from_array(c, 5)};
    m = merge_k(ls, 3);
    print("merge k=3", m);
    free_list(m);
    print("merge NULL", merge(NULL, NULL));
    return 0;
}
