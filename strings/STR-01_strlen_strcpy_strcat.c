/*
 * STR-01: Implement strlen, strcpy, strncpy, strcat, strncat
 * Interview points: '\0' terminator, strcpy has no bounds check (overflow!),
 * strncpy does NOT null-terminate if src is too long and pads with zeros,
 * strcat is O(len(dst)+len(src)) - repeated strcat in a loop is O(n^2).
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>

size_t my_strlen(const char *s)
{
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

char *my_strcpy(char *d, const char *s)
{
    char *r = d;
    while ((*d++ = *s++) != '\0')
        ;
    return r;
}

char *my_strncpy(char *d, const char *s, size_t n)   /* standard semantics */
{
    size_t i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = '\0';
    return d;
}

char *my_strcat(char *d, const char *s)
{
    my_strcpy(d + my_strlen(d), s);
    return d;
}

char *my_strncat(char *d, const char *s, size_t n)   /* always terminates */
{
    char *p = d + my_strlen(d);
    while (n-- && *s) *p++ = *s++;
    *p = '\0';
    return d;
}

/* Safe copy: always terminates, returns length of src (like BSD strlcpy) */
size_t my_strlcpy(char *d, const char *s, size_t size)
{
    size_t len = my_strlen(s);
    if (size) {
        size_t n = len < size - 1 ? len : size - 1;
        memcpy(d, s, n);
        d[n] = '\0';
    }
    return len;
}

int main(void)
{
    char buf[32];
    char small[5];

    printf("strlen(\"hello\") = %u\n", (unsigned)my_strlen("hello"));
    my_strcpy(buf, "Hello");
    my_strcat(buf, ", ");
    my_strncat(buf, "World!!!", 6);
    printf("strcpy+strcat+strncat = \"%s\" %s\n", buf, strcmp(buf, "Hello, World!") ? "FAIL" : "PASS");

    my_strncpy(small, "truncate", sizeof small);   /* NOT terminated! */
    small[sizeof small - 1] = '\0';                /* must do this yourself */
    printf("strncpy then manual terminate = \"%s\"\n", small);

    printf("strlcpy returns %u, copied \"%s\"\n", (unsigned)my_strlcpy(small, "truncate", sizeof small), small);
    return 0;
}
