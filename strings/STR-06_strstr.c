/*
 * STR-06: Implement strstr (find substring)
 *  - brute force O(n*m) - expected first answer
 *  - KMP O(n+m) - mention it, implement if asked (prefix/failure table)
 * Edge case: empty needle returns haystack.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *my_strstr(const char *h, const char *n)
{
    if (!*n) return (char *)h;
    for (; *h; h++) {
        const char *a = h, *b = n;
        while (*a && *b && *a == *b) { a++; b++; }
        if (!*b) return (char *)h;
    }
    return NULL;
}

int kmp_search(const char *t, const char *p)   /* returns index or -1 */
{
    int n = (int)strlen(t), m = (int)strlen(p), i, k = 0, res = -1;
    int *lps;
    if (m == 0) return 0;
    lps = malloc(m * sizeof *lps);
    if (!lps) return -1;
    lps[0] = 0;
    for (i = 1; i < m; i++) {              /* longest proper prefix that is also suffix */
        while (k && p[i] != p[k]) k = lps[k - 1];
        if (p[i] == p[k]) k++;
        lps[i] = k;
    }
    for (i = 0, k = 0; i < n; i++) {
        while (k && t[i] != p[k]) k = lps[k - 1];
        if (t[i] == p[k]) k++;
        if (k == m) { res = i - m + 1; break; }
    }
    free(lps);
    return res;
}

int main(void)
{
    const char *h = "embedded systems interview";
    const char *needles[] = {"systems", "inter", "xyz", "", "w"};
    unsigned i;
    for (i = 0; i < 5; i++) {
        char *r = my_strstr(h, needles[i]);
        printf("strstr(\"%s\") -> %s | kmp idx=%d | libc %s\n", needles[i], r ? r : "NULL",
               kmp_search(h, needles[i]), r == strstr(h, needles[i]) ? "PASS" : "FAIL");
    }
    printf("kmp(\"aaaab\",\"aab\") = %d (expect 2)\n", kmp_search("aaaab", "aab"));
    return 0;
}
