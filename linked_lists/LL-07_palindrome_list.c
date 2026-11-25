/*
 * LL-07: Is a linked list a palindrome? O(n) time, O(1) space
 * 1) find middle (slow/fast)  2) reverse second half  3) compare  4) restore list.
 * Restoring is a nice touch interviewers like - don't leave input modified.
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
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }
static void print(const node_t *h) { for (; h; h = h->next) printf("%d ", h->val); }

static node_t *reverse(node_t *h)
{
    node_t *p = NULL;
    while (h) { node_t *n = h->next; h->next = p; p = h; h = n; }
    return p;
}

int is_palindrome(node_t *h)
{
    node_t *slow = h, *fast = h, *second, *p, *q;
    int ok = 1;
    if (!h || !h->next) return 1;
    while (fast->next && fast->next->next) { slow = slow->next; fast = fast->next->next; }
    second = reverse(slow->next);
    for (p = h, q = second; q; p = p->next, q = q->next)
        if (p->val != q->val) { ok = 0; break; }
    slow->next = reverse(second);          /* restore original list */
    return ok;
}

int main(void)
{
    int t1[] = {1, 2, 3, 2, 1}, t2[] = {1, 2, 2, 1}, t3[] = {1, 2, 3}, t4[] = {7};
    const int *tests[] = {t1, t2, t3, t4};
    int sizes[] = {5, 4, 3, 1}, i;
    for (i = 0; i < 4; i++) {
        node_t *h = from_array(tests[i], sizes[i]);
        int r = is_palindrome(h);
        print(h);
        printf("-> %s (list restored)\n", r ? "palindrome" : "not palindrome");
        free_list(h);
    }
    return 0;
}
