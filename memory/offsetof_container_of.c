/*
 * Implement offsetof() and container_of() (Linux kernel style)
 * offsetof(T, m): address of member m in a struct placed at address 0.
 * container_of(ptr, T, m): given a pointer to a member, get the enclosing struct.
 * Used by intrusive linked lists (Linux list_head), drivers, RTOS kernels.
 * Interview points: why cast to char* (byte arithmetic), size_t result,
 * the "null pointer" trick is technically UB -> real code uses __builtin_offsetof.
 */
#include <stdio.h>
#include <stddef.h>

#define MY_OFFSETOF(type, member) ((size_t)&(((type *)0)->member))

#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - MY_OFFSETOF(type, member)))

/* Intrusive list node embedded in the user struct */
struct list_node { struct list_node *next; };

struct sensor {
    char name[10];
    int id;
    double value;
    struct list_node node;
};

int main(void)
{
    struct sensor s[3] = {
        {"temp", 1, 25.5, {NULL}}, {"press", 2, 1013.2, {NULL}}, {"humid", 3, 40.0, {NULL}},
    };
    struct list_node *it;

    printf("offsetof(id)    mine=%u std=%u\n", (unsigned)MY_OFFSETOF(struct sensor, id),
           (unsigned)offsetof(struct sensor, id));
    printf("offsetof(value) mine=%u std=%u\n", (unsigned)MY_OFFSETOF(struct sensor, value),
           (unsigned)offsetof(struct sensor, value));
    printf("offsetof(node)  mine=%u std=%u\n", (unsigned)MY_OFFSETOF(struct sensor, node),
           (unsigned)offsetof(struct sensor, node));

    s[0].node.next = &s[1].node;               /* build a list through embedded nodes */
    s[1].node.next = &s[2].node;

    printf("Walking intrusive list with container_of:\n");
    for (it = &s[0].node; it; it = it->next) {
        struct sensor *p = CONTAINER_OF(it, struct sensor, node);
        printf("  id=%d name=%-6s value=%.1f\n", p->id, p->name, p->value);
    }

    int *pid = &s[1].id;
    printf("container_of(&s[1].id) == &s[1] -> %s\n",
           CONTAINER_OF(pid, struct sensor, id) == &s[1] ? "PASS" : "FAIL");
    return 0;
}
