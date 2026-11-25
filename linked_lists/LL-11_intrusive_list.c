/*
 * LL-11: Intrusive circular doubly linked list (Linux kernel list_head / RTOS style)
 * The list node lives INSIDE the user struct; container_of gets the struct back.
 * Advantages: no separate allocation per node, one object can sit on several lists
 * (e.g. a task on the ready list AND a timer list), generic code for all types.
 * Sentinel head that points to itself = empty list -> no NULL checks anywhere.
 */
#include <stdio.h>
#include <stddef.h>

struct list_head { struct list_head *next, *prev; };

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

static void __list_add(struct list_head *n, struct list_head *prev, struct list_head *next)
{
    next->prev = n; n->next = next; n->prev = prev; prev->next = n;
}
static void list_add(struct list_head *n, struct list_head *h)      { __list_add(n, h, h->next); } /* front */
static void list_add_tail(struct list_head *n, struct list_head *h) { __list_add(n, h->prev, h); } /* back  */
static void list_del(struct list_head *e)
{
    e->prev->next = e->next;
    e->next->prev = e->prev;
    e->next = e->prev = e;
}
static int list_empty(const struct list_head *h) { return h->next == h; }

typedef struct {
    int id;
    int priority;
    struct list_head ready;     /* node for the ready list */
    struct list_head all;       /* node for the "all tasks" list */
} task_t;

int main(void)
{
    struct list_head ready_q = LIST_HEAD_INIT(ready_q);
    struct list_head all_q = LIST_HEAD_INIT(all_q);
    task_t t[4] = {{1, 5, {0}, {0}}, {2, 3, {0}, {0}}, {3, 7, {0}, {0}}, {4, 1, {0}, {0}}};
    struct list_head *pos;
    int i;

    for (i = 0; i < 4; i++) {
        list_add_tail(&t[i].all, &all_q);
        if (i != 2) list_add_tail(&t[i].ready, &ready_q);   /* task 3 is blocked */
    }
    list_add(&t[2].ready, &ready_q);                        /* now unblocked -> front */

    printf("all tasks  :");
    list_for_each(pos, &all_q) printf(" T%d", list_entry(pos, task_t, all)->id);
    printf("\nready queue:");
    list_for_each(pos, &ready_q) {
        task_t *tk = list_entry(pos, task_t, ready);
        printf(" T%d(p%d)", tk->id, tk->priority);
    }
    list_del(&t[0].ready);
    printf("\nafter T1 blocks:");
    list_for_each(pos, &ready_q) printf(" T%d", list_entry(pos, task_t, ready)->id);
    printf("\nempty? %d\n", list_empty(&ready_q));
    return 0;
}
