/*
 * LL-10: Add two numbers stored as linked lists (digits in reverse order)
 *   (2 -> 4 -> 3) + (5 -> 6 -> 4) = 7 -> 0 -> 8     i.e. 342 + 465 = 807
 * Handle different lengths and the final carry. Follow-up: digits in forward order
 * -> reverse both, add, reverse result (shown).
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *from_digits(const char *s)    /* "342" -> 2 -> 4 -> 3 (reverse order) */
{
    node_t *h = NULL;
    for (; *s; s++) {
        node_t *n = malloc(sizeof *n);
        n->val = *s - '0';
        n->next = h;
        h = n;
    }
    return h;
}
static void print(const char *tag, const node_t *h) { printf("%-10s:", tag); for (; h; h = h->next) printf(" %d", h->val); printf("\n"); }
static void free_list(node_t *h) { while (h) { node_t *n = h->next; free(h); h = n; } }
static node_t *reverse(node_t *h) { node_t *p = NULL; while (h) { node_t *n = h->next; h->next = p; p = h; h = n; } return p; }

node_t *add_lists(const node_t *a, const node_t *b)
{
    node_t dummy = {0, NULL}, *t = &dummy;
    int carry = 0;
    while (a || b || carry) {
        int sum = carry + (a ? a->val : 0) + (b ? b->val : 0);
        t->next = malloc(sizeof *t->next);
        t = t->next;
        t->val = sum % 10;
        t->next = NULL;
        carry = sum / 10;
        if (a) a = a->next;
        if (b) b = b->next;
    }
    return dummy.next;
}

int main(void)
{
    node_t *a = from_digits("342"), *b = from_digits("465"), *r;
    print("342", a); print("465", b);
    r = add_lists(a, b); print("sum (rev)", r);
    free_list(a); free_list(b); free_list(r);

    a = from_digits("9999"); b = from_digits("1");
    r = add_lists(a, b); print("9999+1", r);       /* carry creates a new digit */
    r = reverse(r);      print("forward", r);
    free_list(a); free_list(b); free_list(r);
    return 0;
}
