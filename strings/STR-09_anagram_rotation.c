/*
 * STR-09: Anagram check and string rotation check
 * Anagram: same character counts -> one int[256] table, ++ for a, -- for b. O(n).
 * Rotation: b is a rotation of a  <=>  same length AND b is a substring of a+a.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int is_anagram(const char *a, const char *b)
{
    int cnt[256] = {0}, i;
    if (strlen(a) != strlen(b)) return 0;
    for (; *a; a++, b++) {
        cnt[(unsigned char)*a]++;
        cnt[(unsigned char)*b]--;
    }
    for (i = 0; i < 256; i++) if (cnt[i]) return 0;
    return 1;
}

int is_rotation(const char *a, const char *b)
{
    size_t n = strlen(a);
    char *aa;
    int r;
    if (n != strlen(b)) return 0;
    aa = malloc(2 * n + 1);
    if (!aa) return 0;
    memcpy(aa, a, n);
    memcpy(aa + n, a, n + 1);
    r = strstr(aa, b) != NULL;
    free(aa);
    return r;
}

int main(void)
{
    printf("is_anagram(listen, silent)=%d (hello, world)=%d\n", is_anagram("listen", "silent"), is_anagram("hello", "world"));
    printf("is_rotation(waterbottle, erbottlewat)=%d (abc, acb)=%d\n",
           is_rotation("waterbottle", "erbottlewat"), is_rotation("abc", "acb"));
    return 0;
}
