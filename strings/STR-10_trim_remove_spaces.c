/*
 * STR-10: Trim whitespace, remove all spaces, collapse multiple spaces, remove a char
 * Pattern for all in-place filters: read pointer + write pointer, O(n), O(1) space.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *trim(char *s)                 /* returns pointer into s, modifies the end */
{
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (!*s) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

void remove_char(char *s, char c)
{
    char *w = s;
    for (; *s; s++) if (*s != c) *w++ = *s;
    *w = '\0';
}

void collapse_spaces(char *s)
{
    char *start = s, *w = s;
    int prev_space = 1;             /* also drops leading spaces */
    for (; *s; s++) {
        if (*s == ' ') {
            if (!prev_space) *w++ = ' ';
            prev_space = 1;
        } else {
            *w++ = *s;
            prev_space = 0;
        }
    }
    if (w > start && w[-1] == ' ') w--;   /* drop trailing space */
    *w = '\0';
}

int main(void)
{
    char a[] = "   hello world \t\n";
    char b[] = " r e m o v e   s p a c e s ";
    char c[] = "   too    many   spaces  ";

    printf("trim            : [%s]\n", trim(a));
    remove_char(b, ' ');
    printf("remove spaces   : [%s]\n", b);
    collapse_spaces(c);
    printf("collapse spaces : [%s] %s\n", c, strcmp(c, "too many spaces") ? "FAIL" : "PASS");
    return 0;
}
