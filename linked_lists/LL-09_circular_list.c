/*
 * LL-09: Circular linked list
 *  - keep a TAIL pointer: tail->next is the head, so insert at both ends is O(1)
 *  - traversal must stop when you get back to the start (do-while)
 *  - Josephus problem: every k-th person is eliminated, who survives?
 * Used for: round-robin task scheduling, circular playlists.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct node { int val; struct node *next; } node_t;

node_t *insert_front(node_t *tail, int v)   /* returns tail */
{
    node_t *n = malloc(sizeof *n);
    n->val = v;
    if (!tail) { n->next = n; return n; }
    n->next = tail->next;
    tail->next = n;
    return tail;
}

node_t *append(node_t *tail, int v)         /* insert at back, O(1) */
{
    tail = insert_front(tail, v);           /* new node sits right after tail... */
    return tail->next;                      /* ...so just make it the new tail */
}

node_t *delete_value(node_t *tail, int v)
{
    node_t *prev = tail, *cur;
    if (!tail) return NULL;
    do {
        cur = prev->next;
        if (cur->val == v) {
            if (cur == prev) { free(cur); return NULL; }   /* only node */
            prev->next = cur->next;
            if (cur == tail) tail = prev;
            free(cur);
            return tail;
        }
        prev = cur;
    } while (prev != tail);
    return tail;
}

static void print(const char *s, const node_t *tail)
{
    const node_t *p;
    printf("%-14s:", s);
    if (!tail) { printf(" (empty)\n"); return; }
    p = tail->next;
    do { printf(" %d", p->val); p = p->next; } while (p != tail->next);
    printf("\n");
}

int josephus(int n, int k)
{
    node_t *tail = NULL, *prev;
    int i, survivor;
    for (i = 1; i <= n; i++) tail = append(tail, i);
    prev = tail;
    while (prev->next != prev) {
        node_t *dead;
        for (i = 1; i < k; i++) prev = prev->next;
        dead = prev->next;
        prev->next = dead->next;
        free(dead);
    }
    survivor = prev->val;
    free(prev);
    return survivor;
}

int main(void)
{
    node_t *t = NULL;
    t = append(t, 1); t = append(t, 2); t = append(t, 3);
    t = insert_front(t, 0);
    print("0 | 1 2 3", t);
    t = delete_value(t, 0);  print("delete head 0", t);
    t = delete_value(t, 3);  print("delete tail 3", t);
    t = delete_value(t, 1);
    t = delete_value(t, 2);  print("delete all", t);
    printf("josephus(7, 3) survivor = %d (expect 4)\n", josephus(7, 3));
    printf("josephus(5, 2) survivor = %d (expect 3)\n", josephus(5, 2));
    return 0;
}
