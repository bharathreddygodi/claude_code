/*
 * PTR-02: const and pointers - read declarations RIGHT TO LEFT
 *   const int *p;        p is a pointer to const int  -> *p read-only, p can move
 *   int const *p;        same as above
 *   int *const p;        p is a const pointer to int  -> p fixed, *p writable
 *   const int *const p;  both fixed
 * Embedded: const data goes to flash; "volatile const" = read-only HW status register.
 * Uncomment the ERROR lines to see the compiler errors.
 */
#include <stdio.h>

static void print_buf(const char *buf)   /* promise: function won't modify data */
{
    printf("%s\n", buf);
}

int main(void)
{
    int a = 1, b = 2;

    const int *p1 = &a;      /* pointer to const */
    p1 = &b;                 /* OK  */
    /* *p1 = 5; */           /* ERROR: assignment of read-only location */

    int *const p2 = &a;      /* const pointer */
    *p2 = 5;                 /* OK  */
    /* p2 = &b; */           /* ERROR: assignment of read-only variable */

    const int *const p3 = &b;
    /* *p3 = 1; p3 = &a; */  /* both ERROR */

    printf("*p1=%d *p2=%d (a=%d) *p3=%d\n", *p1, *p2, a, *p3);
    print_buf("const char* param accepts any string");

    /* Casting away const and writing is UB if the object was really defined const */
    return 0;
}
