/*
 * PTR-10: Bad pointers and how to avoid them
 *  - wild pointer:     uninitialised pointer -> initialise to NULL
 *  - dangling pointer: points to freed memory or a dead stack frame
 *  - returning address of a local variable (classic bug) vs static vs malloc
 *  - NULL dereference, use-after-free, double free
 *  - restrict: promise that pointers don't alias -> lets compiler optimise
 * The buggy code is shown in comments only so this file runs cleanly.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* WRONG: returns pointer to stack memory that dies when the function returns
char *get_name_bad(void) { char buf[16] = "sensor"; return buf; }
*/
const char *get_name_literal(void) { return "sensor"; }          /* OK: literal lives forever */
char *get_name_static(void) { static char buf[16] = "sensor"; return buf; } /* OK, not reentrant */
char *get_name_heap(void)                                         /* OK, caller frees */
{
    char *p = malloc(16);
    if (p) strcpy(p, "sensor");
    return p;
}
void get_name_caller_buf(char *out, size_t n) { snprintf(out, n, "sensor"); } /* best for embedded */

void add_arrays(int *restrict dst, const int *restrict a, const int *restrict b, int n)
{
    int i;
    for (i = 0; i < n; i++) dst[i] = a[i] + b[i];
}

int main(void)
{
    int *wild = NULL;                /* never leave a pointer uninitialised */
    char buf[16];
    char *h;
    int x[3] = {1, 2, 3}, y[3] = {10, 20, 30}, z[3];

    printf("literal: %s, static: %s\n", get_name_literal(), get_name_static());
    h = get_name_heap();
    printf("heap: %s\n", h);
    free(h);
    h = NULL;                        /* prevent dangling use / double free */
    get_name_caller_buf(buf, sizeof buf);
    printf("caller buffer: %s\n", buf);

    if (wild == NULL) printf("wild is NULL - checked before use\n");
    free(h);                         /* free(NULL) is safe - no double free */

    add_arrays(z, x, y, 3);
    printf("restrict add: %d %d %d\n", z[0], z[1], z[2]);

    /* Detect these bugs: gcc -fsanitize=address -g file.c  (Linux/Clang), or valgrind */
    return 0;
}
