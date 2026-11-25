/*
 * LL-06: Find where two lists intersect (Y-shaped lists share a tail)
 *  1) length difference: advance the longer by |la - lb|, then walk together
 *  2) two-pointer switch: when a pointer hits the end, jump to the OTHER head;
 *     both travel la + lb steps and meet at the intersection (or both NULL).
 * Intersection is by node ADDRESS, not value.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *mk(int v, node_t *next) { node_t *n = malloc(sizeof *n); n->val = v; n->next = next; return n; }
static int len(const node_t *h) { int n = 0; for (; h; h = h->next) n++; return n; }

node_t *intersect_len(node_t *a, node_t *b)
{
    int la = len(a), lb = len(b);
    while (la > lb) { a = a->next; la--; }
    while (lb > la) { b = b->next; lb--; }
    while (a != b) { a = a->next; b = b->next; }
    return a;
}

node_t *intersect_switch(node_t *a, node_t *b)
{
    node_t *p = a, *q = b;
    while (p != q) {
        p = p ? p->next : b;
        q = q ? q->next : a;
    }
    return p;
}

int main(void)
{
    node_t *common = mk(8, mk(9, mk(10, NULL)));
    node_t *a = mk(1, mk(2, mk(3, common)));      /* 1 2 3 8 9 10 */
    node_t *b = mk(6, common);                    /* 6 8 9 10     */
    node_t *c = mk(5, mk(4, NULL));               /* separate     */
    node_t *r;

    r = intersect_len(a, b);
    printf("length method : %d\n", r ? r->val : -1);
    r = intersect_switch(a, b);
    printf("switch method : %d\n", r ? r->val : -1);
    r = intersect_switch(a, c);
    printf("no intersection -> %s\n", r ? "FAIL" : "NULL (PASS)");
    /* (nodes intentionally not freed - shared tail makes freeing need care) */
    return 0;
}
