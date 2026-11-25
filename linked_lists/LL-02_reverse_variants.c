/*
 * LL-02: Reverse a linked list - iterative, recursive, in groups of K, between m..n
 * Iterative is O(n)/O(1) - the expected answer. Recursive is O(n) stack (risky on MCUs).
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *build(int n)
{
    node_t *h = NULL, **t = &h;
    int i;
    for (i = 1; i <= n; i++) {
        *t = malloc(sizeof **t);
        (*t)->val = i; (*t)->next = NULL;
        t = &(*t)->next;
    }
    return h;
}
static void print(const char *s, const node_t *h) { printf("%-16s:", s); for (; h; h = h->next) printf(" %d", h->val); printf("\n"); }
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }

node_t *rev_iter(node_t *h)
{
    node_t *prev = NULL;
    while (h) { node_t *nx = h->next; h->next = prev; prev = h; h = nx; }
    return prev;
}

node_t *rev_rec(node_t *h)
{
    node_t *rest;
    if (!h || !h->next) return h;
    rest = rev_rec(h->next);
    h->next->next = h;          /* the node after h now points back to h */
    h->next = NULL;
    return rest;
}

node_t *rev_k_group(node_t *h, int k)   /* leftover < k nodes stay as they are */
{
    node_t *cur = h, *prev = NULL, *nx = NULL;
    int i;
    for (i = 0; i < k; i++) { if (!cur) return h; cur = cur->next; }
    cur = h;
    for (i = 0; i < k; i++) { nx = cur->next; cur->next = prev; prev = cur; cur = nx; }
    h->next = rev_k_group(cur, k);      /* h is now the tail of this group */
    return prev;
}

node_t *rev_between(node_t *h, int m, int n)  /* 1-based positions */
{
    node_t dummy = {0, h}, *pre = &dummy, *cur;
    int i;
    for (i = 1; i < m; i++) pre = pre->next;
    cur = pre->next;
    for (i = 0; i < n - m; i++) {       /* move cur->next to front of the sublist */
        node_t *mv = cur->next;
        cur->next = mv->next;
        mv->next = pre->next;
        pre->next = mv;
    }
    return dummy.next;
}

int main(void)
{
    node_t *h = build(8);
    print("original", h);
    h = rev_iter(h);         print("iterative", h);
    h = rev_rec(h);          print("recursive (back)", h);
    h = rev_k_group(h, 3);   print("k-group k=3", h);
    free_list(h);
    h = rev_between(build(7), 2, 5);
    print("between 2..5", h);
    free_list(h);
    return 0;
}
