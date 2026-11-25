/*
 * LL-05: Remove duplicates
 *  - sorted list: compare with next, O(n)
 *  - unsorted list, no extra memory: runner technique, O(n^2)
 *  - unsorted list with a hash/bitmap of seen values, O(n) (small value range here)
 *  - remove all nodes with a given value
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static void print(const char *s, const node_t *h) { printf("%-20s:", s); for (; h; h = h->next) printf(" %d", h->val); printf("\n"); }
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }

void dedup_sorted(node_t *h)
{
    while (h && h->next) {
        if (h->val == h->next->val) {
            node_t *d = h->next;
            h->next = d->next;
            free(d);
        } else {
            h = h->next;
        }
    }
}

void dedup_unsorted_runner(node_t *h)
{
    for (; h; h = h->next) {
        node_t *r = h;
        while (r->next) {
            if (r->next->val == h->val) { node_t *d = r->next; r->next = d->next; free(d); }
            else r = r->next;
        }
    }
}

void dedup_unsorted_seen(node_t *h)   /* values assumed 0..255 */
{
    unsigned char seen[256];
    node_t *prev = NULL;
    memset(seen, 0, sizeof seen);
    while (h) {
        if (seen[h->val & 0xFF]) { prev->next = h->next; free(h); h = prev->next; }
        else { seen[h->val & 0xFF] = 1; prev = h; h = h->next; }
    }
}

void remove_all(node_t **pp, int v)
{
    while (*pp) {
        if ((*pp)->val == v) { node_t *d = *pp; *pp = d->next; free(d); }
        else pp = &(*pp)->next;
    }
}

int main(void)
{
    int s[] = {1, 1, 2, 3, 3, 3, 4, 5, 5};
    int u[] = {4, 2, 4, 1, 2, 7, 1, 4};
    node_t *h;

    h = from_array(s, 9); dedup_sorted(h);          print("sorted dedup", h); free_list(h);
    h = from_array(u, 8); dedup_unsorted_runner(h); print("unsorted (runner)", h); free_list(h);
    h = from_array(u, 8); dedup_unsorted_seen(h);   print("unsorted (seen[])", h); free_list(h);
    h = from_array(u, 8); remove_all(&h, 4);        print("remove all 4s", h); free_list(h);
    return 0;
}
