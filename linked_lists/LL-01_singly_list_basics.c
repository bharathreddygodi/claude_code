/*
 * LL-01: Singly linked list - full CRUD
 * insert at head / tail / position, delete by value / position, search, length, free.
 * Interview points: always handle empty list, head change (use node** or return new
 * head), position out of range, free every node, check malloc.
 * (Reverse / middle / loop detection / Nth-from-end are in pointers/PTR-09.)
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

static node_t *new_node(int v)
{
    node_t *n = malloc(sizeof *n);
    if (n) { n->val = v; n->next = NULL; }
    return n;
}

void insert_head(node_t **head, int v)
{
    node_t *n = new_node(v);
    if (!n) return;
    n->next = *head;
    *head = n;
}

void insert_tail(node_t **head, int v)
{
    while (*head) head = &(*head)->next;     /* walk to the last link */
    *head = new_node(v);
}

int insert_at(node_t **head, int pos, int v)  /* pos 0 = head; returns -1 if out of range */
{
    node_t *n;
    while (pos-- > 0) {
        if (!*head) return -1;
        head = &(*head)->next;
    }
    n = new_node(v);
    if (!n) return -1;
    n->next = *head;
    *head = n;
    return 0;
}

int delete_value(node_t **head, int v)
{
    while (*head && (*head)->val != v) head = &(*head)->next;
    if (!*head) return -1;
    node_t *dead = *head;
    *head = dead->next;
    free(dead);
    return 0;
}

int delete_at(node_t **head, int pos)
{
    while (pos-- > 0 && *head) head = &(*head)->next;
    if (!*head) return -1;
    node_t *dead = *head;
    *head = dead->next;
    free(dead);
    return 0;
}

int search(const node_t *h, int v)            /* index or -1 */
{
    int i;
    for (i = 0; h; h = h->next, i++) if (h->val == v) return i;
    return -1;
}

int length(const node_t *h) { int n = 0; for (; h; h = h->next) n++; return n; }

void free_list(node_t **head)
{
    while (*head) { node_t *n = (*head)->next; free(*head); *head = n; }
}

static void print(const char *tag, const node_t *h)
{
    int n = length(h);
    printf("%-22s:", tag);
    for (; h; h = h->next) printf(" %d", h->val);
    printf("  (len %d)\n", n);
}

int main(void)
{
    node_t *h = NULL;
    insert_tail(&h, 20); insert_tail(&h, 30); insert_head(&h, 10);
    print("tail 20,30 head 10", h);
    insert_at(&h, 2, 25);           print("insert 25 at 2", h);
    insert_at(&h, 0, 5);            print("insert 5 at 0", h);
    printf("insert at 99 -> %d (out of range)\n", insert_at(&h, 99, 1));
    delete_value(&h, 5);            print("delete value 5 (head)", h);
    delete_at(&h, 3);               print("delete at 3 (tail)", h);
    printf("search 25 -> %d, search 7 -> %d\n", search(h, 25), search(h, 7));
    free_list(&h);
    printf("after free head=%p\n", (void *)h);
    return 0;
}
