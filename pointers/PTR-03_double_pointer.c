/*
 * PTR-03: Pointer to pointer
 *  - a function that must CHANGE the caller's pointer needs T** (C is pass-by-value)
 *  - linked list insert/delete at head with node**  (no special case for head!)
 *  - char **argv / array of strings
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void alloc_wrong(int *p)  { p = malloc(sizeof *p); (void)p; }   /* leaks; caller sees nothing */
int  alloc_right(int **pp)
{
    *pp = malloc(sizeof **pp);
    if (!*pp) return -1;
    **pp = 99;
    return 0;
}

typedef struct node { int val; struct node *next; } node_t;

void push(node_t **head, int v)
{
    node_t *n = malloc(sizeof *n);
    if (!n) return;
    n->val = v;
    n->next = *head;
    *head = n;
}

/* Linus Torvalds' "good taste" delete: walk a pointer-to-the-link */
void remove_val(node_t **head, int v)
{
    node_t **pp = head;
    while (*pp && (*pp)->val != v)
        pp = &(*pp)->next;
    if (*pp) {
        node_t *dead = *pp;
        *pp = dead->next;
        free(dead);
    }
}

static void print_list(const node_t *n)
{
    for (; n; n = n->next) printf("%d -> ", n->val);
    printf("NULL\n");
}

int main(void)
{
    int *p = NULL;
    node_t *head = NULL;
    const char *names[] = {"uart", "spi", "i2c", NULL};
    const char **it;

    alloc_wrong(p);
    printf("after alloc_wrong p=%p (still NULL)\n", (void *)p);
    alloc_right(&p);
    printf("after alloc_right *p=%d\n", *p);
    free(p);

    push(&head, 3); push(&head, 2); push(&head, 1);
    print_list(head);
    remove_val(&head, 1);   /* head removal - no special case */
    remove_val(&head, 3);
    print_list(head);
    remove_val(&head, 2);

    for (it = names; *it; it++) printf("%s ", *it);
    printf("\n");
    return 0;
}
