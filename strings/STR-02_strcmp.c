/*
 * STR-02: Implement strcmp, strncmp, case-insensitive strcasecmp
 * Compare as unsigned char; return the difference at the first mismatch.
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

int my_strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int my_strncmp(const char *a, const char *b, size_t n)
{
    for (; n; n--, a++, b++) {
        if (*a != *b || !*a)
            return (unsigned char)*a - (unsigned char)*b;
    }
    return 0;
}

int my_strcasecmp(const char *a, const char *b)
{
    int ca, cb;
    do {
        ca = tolower((unsigned char)*a++);
        cb = tolower((unsigned char)*b++);
    } while (ca && ca == cb);
    return ca - cb;
}

static int sgn(int x) { return (x > 0) - (x < 0); }

int main(void)
{
    const char *t[][2] = {{"abc", "abc"}, {"abc", "abd"}, {"b", "abc"}, {"", "a"}, {"ab", "abc"}};
    unsigned i;
    for (i = 0; i < 5; i++)
        printf("strcmp(\"%s\",\"%s\") mine=%2d libc=%2d %s\n", t[i][0], t[i][1],
               sgn(my_strcmp(t[i][0], t[i][1])), sgn(strcmp(t[i][0], t[i][1])),
               sgn(my_strcmp(t[i][0], t[i][1])) == sgn(strcmp(t[i][0], t[i][1])) ? "PASS" : "FAIL");
    printf("strncmp(\"abcX\",\"abcY\",3) = %d\n", my_strncmp("abcX", "abcY", 3));
    printf("strcasecmp(\"HeLLo\",\"hello\") = %d\n", my_strcasecmp("HeLLo", "hello"));
    return 0;
}
