/*
 * LL-08: Doubly linked list - insert front/back/after, delete node in O(1), reverse
 * With head AND tail pointers, push/pop at both ends are O(1) (used as a deque).
 * Delete given a node pointer is O(1) - the big advantage over singly linked.
 * Cost: extra pointer per node, more links to keep consistent.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct dnode { int val; struct dnode *prev, *next; } dnode_t;
typedef struct { dnode_t *head, *tail; int size; } dlist_t;

static dnode_t *mk(int v) { dnode_t *n = calloc(1, sizeof *n); if (n) n->val = v; return n; }

void push_front(dlist_t *l, int v)
{
    dnode_t *n = mk(v);
    if (!n) return;
    n->next = l->head;
    if (l->head) l->head->prev = n; else l->tail = n;
    l->head = n;
    l->size++;
}

void push_back(dlist_t *l, int v)
{
    dnode_t *n = mk(v);
    if (!n) return;
    n->prev = l->tail;
    if (l->tail) l->tail->next = n; else l->head = n;
    l->tail = n;
    l->size++;
}

void insert_after(dlist_t *l, dnode_t *at, int v)
{
    dnode_t *n;
    if (at == l->tail) { push_back(l, v); return; }
    n = mk(v);
    if (!n) return;
    n->prev = at;
    n->next = at->next;
    at->next->prev = n;
    at->next = n;
    l->size++;
}

void remove_node(dlist_t *l, dnode_t *n)          /* O(1) */
{
    if (n->prev) n->prev->next = n->next; else l->head = n->next;
    if (n->next) n->next->prev = n->prev; else l->tail = n->prev;
    free(n);
    l->size--;
}

void reverse(dlist_t *l)
{
    dnode_t *cur = l->head, *tmp;
    while (cur) {                                  /* swap prev/next of every node */
        tmp = cur->prev; cur->prev = cur->next; cur->next = tmp;
        cur = cur->prev;
    }
    tmp = l->head; l->head = l->tail; l->tail = tmp;
}

static void print(const char *s, const dlist_t *l)
{
    const dnode_t *n;
    printf("%-18s fwd:", s);
    for (n = l->head; n; n = n->next) printf(" %d", n->val);
    printf("  bwd:");
    for (n = l->tail; n; n = n->prev) printf(" %d", n->val);
    printf("  size=%d\n", l->size);
}

int main(void)
{
    dlist_t l = {NULL, NULL, 0};
    push_back(&l, 2); push_back(&l, 3); push_front(&l, 1);
    print("push 1,2,3", &l);
    insert_after(&l, l.head, 9);     print("insert 9 after 1", &l);
    remove_node(&l, l.head->next);   print("remove 9 (O(1))", &l);
    remove_node(&l, l.tail);         print("remove tail", &l);
    push_back(&l, 4); push_back(&l, 5);
    reverse(&l);                     print("reverse", &l);
    while (l.head) remove_node(&l, l.head);
    print("empty", &l);
    return 0;
}
