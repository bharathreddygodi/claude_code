/*
 * LL-12: LRU cache - get/put in O(1)
 * Hash table (key -> node) + doubly linked list ordered by recency.
 *   get: look up, move node to front.
 *   put: update or insert at front; if over capacity evict the tail (least recent).
 * Static node pool so it's embedded-friendly (no malloc after init).
 */
#include <stdio.h>
#include <string.h>

#define CAP      3
#define NBUCKETS 16

typedef struct lnode {
    int key, val;
    struct lnode *prev, *next;      /* recency list */
    struct lnode *hnext;            /* hash chain */
} lnode_t;

typedef struct {
    lnode_t pool[CAP];
    int used;
    lnode_t *bucket[NBUCKETS];
    lnode_t head, tail;             /* sentinels: head.next = most recent */
} lru_t;

static unsigned hash(int k) { return (unsigned)k % NBUCKETS; }

static void unlink_node(lnode_t *n) { n->prev->next = n->next; n->next->prev = n->prev; }
static void push_front(lru_t *c, lnode_t *n)
{
    n->next = c->head.next; n->prev = &c->head;
    c->head.next->prev = n; c->head.next = n;
}

static lnode_t *find(lru_t *c, int key)
{
    lnode_t *n;
    for (n = c->bucket[hash(key)]; n; n = n->hnext) if (n->key == key) return n;
    return NULL;
}

static void hash_remove(lru_t *c, lnode_t *n)
{
    lnode_t **pp = &c->bucket[hash(n->key)];
    while (*pp != n) pp = &(*pp)->hnext;
    *pp = n->hnext;
}

void lru_init(lru_t *c)
{
    memset(c, 0, sizeof *c);
    c->head.next = &c->tail;
    c->tail.prev = &c->head;
}

int lru_get(lru_t *c, int key, int *val)
{
    lnode_t *n = find(c, key);
    if (!n) return -1;
    unlink_node(n);
    push_front(c, n);
    *val = n->val;
    return 0;
}

void lru_put(lru_t *c, int key, int val)
{
    lnode_t *n = find(c, key);
    if (n) { n->val = val; unlink_node(n); push_front(c, n); return; }
    if (c->used < CAP) {
        n = &c->pool[c->used++];
    } else {                                   /* evict least recently used */
        n = c->tail.prev;
        printf("  evict key %d\n", n->key);
        unlink_node(n);
        hash_remove(c, n);
    }
    n->key = key; n->val = val;
    n->hnext = c->bucket[hash(key)];
    c->bucket[hash(key)] = n;
    push_front(c, n);
}

static void show(lru_t *c)
{
    lnode_t *n;
    printf("  order (MRU->LRU):");
    for (n = c->head.next; n != &c->tail; n = n->next) printf(" %d=%d", n->key, n->val);
    printf("\n");
}

int main(void)
{
    lru_t c;
    int v;
    lru_init(&c);
    lru_put(&c, 1, 100); lru_put(&c, 2, 200); lru_put(&c, 3, 300); show(&c);
    printf("get 1 -> %d\n", lru_get(&c, 1, &v) == 0 ? v : -1); show(&c);
    printf("put 4:\n"); lru_put(&c, 4, 400); show(&c);     /* evicts 2 */
    printf("get 2 -> %d (evicted)\n", lru_get(&c, 2, &v) == 0 ? v : -1);
    lru_put(&c, 3, 333); show(&c);
    return 0;
}
