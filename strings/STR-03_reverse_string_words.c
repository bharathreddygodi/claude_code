/*
 * STR-03: Reverse a string in place, reverse it recursively, reverse words
 * Reverse words: "the sky is blue" -> "blue is sky the"
 *   trick: reverse the whole string, then reverse each word. O(n), O(1) space.
 * Gotcha: char *s = "literal"; s[0] = 'x'; -> UB (literal is read-only).
 *         Use char s[] = "literal"; instead.
 */
#include <stdio.h>
#include <string.h>

void reverse_range(char *a, char *b)    /* reverse [a, b] inclusive */
{
    while (a < b) {
        char t = *a;
        *a++ = *b;
        *b-- = t;
    }
}

void reverse(char *s)
{
    size_t n = strlen(s);
    if (n) reverse_range(s, s + n - 1);
}

void reverse_rec(char *s, int lo, int hi)
{
    if (lo >= hi) return;
    char t = s[lo]; s[lo] = s[hi]; s[hi] = t;
    reverse_rec(s, lo + 1, hi - 1);
}

void reverse_words(char *s)
{
    char *start = s, *p = s;
    reverse(s);
    for (;; p++) {
        if (*p == ' ' || *p == '\0') {
            if (p > start) reverse_range(start, p - 1);
            if (*p == '\0') break;
            start = p + 1;
        }
    }
}

int main(void)
{
    char a[] = "embedded";
    char b[] = "recursion";
    char c[] = "the sky is blue";

    reverse(a);
    printf("reverse        : %s\n", a);
    reverse_rec(b, 0, (int)strlen(b) - 1);
    printf("reverse_rec    : %s\n", b);
    reverse_words(c);
    printf("reverse_words  : %s %s\n", c, strcmp(c, "blue is sky the") ? "FAIL" : "PASS");
    return 0;
}
