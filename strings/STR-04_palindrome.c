/*
 * STR-04: Palindrome checks
 *  - plain string palindrome (two pointers)
 *  - ignoring case and non-alphanumerics ("A man, a plan, a canal: Panama")
 *  - integer palindrome without converting to a string
 *  - longest palindromic substring (expand around centre, O(n^2))
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_pal(const char *s)
{
    size_t i = 0, j = strlen(s);
    if (j == 0) return 1;
    for (j--; i < j; i++, j--)
        if (s[i] != s[j]) return 0;
    return 1;
}

int is_pal_clean(const char *s)
{
    const char *a = s, *b = s + strlen(s) - 1;
    while (a < b) {
        if (!isalnum((unsigned char)*a)) { a++; continue; }
        if (!isalnum((unsigned char)*b)) { b--; continue; }
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b--;
    }
    return 1;
}

int is_pal_int(int x)
{
    int rev = 0;
    if (x < 0 || (x % 10 == 0 && x != 0)) return 0;
    while (x > rev) {               /* reverse only half -> no overflow */
        rev = rev * 10 + x % 10;
        x /= 10;
    }
    return x == rev || x == rev / 10;
}

void longest_pal(const char *s, int *start, int *len)
{
    int n = (int)strlen(s), c, best = 0, bs = 0;
    for (c = 0; c < n; c++) {
        int k;
        for (k = 0; k < 2; k++) {           /* odd (c,c) and even (c,c+1) centres */
            int l = c, r = c + k;
            while (l >= 0 && r < n && s[l] == s[r]) { l--; r++; }
            if (r - l - 1 > best) { best = r - l - 1; bs = l + 1; }
        }
    }
    *start = bs; *len = best;
}

int main(void)
{
    int st, ln;
    printf("is_pal(\"racecar\")=%d is_pal(\"abca\")=%d\n", is_pal("racecar"), is_pal("abca"));
    printf("is_pal_clean(\"A man, a plan, a canal: Panama\")=%d\n", is_pal_clean("A man, a plan, a canal: Panama"));
    printf("is_pal_int(12321)=%d is_pal_int(1221)=%d is_pal_int(123)=%d\n",
           is_pal_int(12321), is_pal_int(1221), is_pal_int(123));
    longest_pal("forgeeksskeegfor", &st, &ln);
    printf("longest palindrome in forgeeksskeegfor = %.*s\n", ln, "forgeeksskeegfor" + st);
    return 0;
}
