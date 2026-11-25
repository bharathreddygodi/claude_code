/*
 * PTR-07: Reading complex declarations (clockwise / right-left rule)
 * Start at the name, go right until ')', then left, repeat.
 *
 *   int *a[5];             a is an array of 5 pointers to int
 *   int (*b)[5];           b is a pointer to an array of 5 ints
 *   int *c(void);          c is a function returning pointer to int
 *   int (*d)(void);        d is a pointer to a function returning int
 *   int (*e[3])(int);      e is an array of 3 pointers to functions(int) returning int
 *   int (*(*f)(void))[5];  f is a pointer to a function returning pointer to array of 5 ints
 *   void (*signal(int, void (*)(int)))(int);   the famous signal() prototype
 * Use typedef to make these readable - shown below for signal().
 */
#include <stdio.h>

static int g_arr[5] = {1, 2, 3, 4, 5};
static int  sq(int x)   { return x * x; }
static int  neg(int x)  { return -x; }
static int  dbl(int x)  { return 2 * x; }
static int *get_ptr(void) { return &g_arr[0]; }
static int  get_one(void) { return 1; }
static int (*get_arr(void))[5] { return &g_arr; }

typedef void (*handler_t)(int);                 /* readable signal() */
static void my_handler(int s) { printf("  handler got %d\n", s); }
static handler_t my_signal(int sig, handler_t h) { h(sig); return h; }

int main(void)
{
    int x = 7, y = 8;
    int *a[5] = {&x, &y};
    int (*b)[5] = &g_arr;
    int *(*c)(void) = get_ptr;
    int (*d)(void) = get_one;
    int (*e[3])(int) = {sq, neg, dbl};
    int (*(*f)(void))[5] = get_arr;

    printf("*a[1]=%d  (*b)[2]=%d  *c()=%d  d()=%d\n", *a[1], (*b)[2], *c(), d());
    printf("e[0](4)=%d e[1](4)=%d e[2](4)=%d\n", e[0](4), e[1](4), e[2](4));
    printf("(*f())[4]=%d\n", (*f())[4]);
    my_signal(11, my_handler);
    return 0;
}
