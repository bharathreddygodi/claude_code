/*
 * STR-08: Character frequency problems (256-entry count table, O(n))
 *  - first non-repeating character
 *  - remove duplicate characters in place (keep first occurrence)
 *  - check if all characters are unique (bitmap / table)
 *  - most frequent character
 */
#include <stdio.h>
#include <string.h>

int first_unique(const char *s)            /* returns index or -1 */
{
    int cnt[256] = {0}, i;
    for (i = 0; s[i]; i++) cnt[(unsigned char)s[i]]++;
    for (i = 0; s[i]; i++) if (cnt[(unsigned char)s[i]] == 1) return i;
    return -1;
}

void remove_dups(char *s)
{
    unsigned char seen[256] = {0};
    char *w = s;
    for (; *s; s++) {
        if (!seen[(unsigned char)*s]) {
            seen[(unsigned char)*s] = 1;
            *w++ = *s;
        }
    }
    *w = '\0';
}

int all_unique(const char *s)
{
    unsigned char seen[256] = {0};
    for (; *s; s++) {
        if (seen[(unsigned char)*s]++) return 0;
    }
    return 1;
}

char most_frequent(const char *s)
{
    int cnt[256] = {0}, best = 0, i;
    char c = 0;
    for (i = 0; s[i]; i++)
        if (++cnt[(unsigned char)s[i]] > best) { best = cnt[(unsigned char)s[i]]; c = s[i]; }
    return c;
}

int main(void)
{
    char s[] = "programming";
    int i = first_unique("swiss");
    printf("first unique in \"swiss\" = '%c' at %d\n", "swiss"[i], i);
    printf("all_unique(\"abcde\")=%d all_unique(\"hello\")=%d\n", all_unique("abcde"), all_unique("hello"));
    printf("most frequent in \"mississippi\" = '%c'\n", most_frequent("mississippi"));
    remove_dups(s);
    printf("remove_dups(\"programming\") = \"%s\" %s\n", s, strcmp(s, "progamin") ? "FAIL" : "PASS");
    return 0;
}
