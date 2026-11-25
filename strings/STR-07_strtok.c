/*
 * STR-07: Implement strtok and a reentrant strtok_r
 * strtok keeps hidden static state -> NOT thread-safe / not reentrant (can't nest).
 * It modifies the input (writes '\0' over delimiters) -> can't use on literals.
 * Typical embedded use: parsing AT commands / CLI input "SET,LED,1".
 */
#include <stdio.h>
#include <string.h>

static int is_delim(char c, const char *d)
{
    for (; *d; d++) if (c == *d) return 1;
    return 0;
}

char *my_strtok_r(char *s, const char *delim, char **save)
{
    char *tok;
    if (!s) s = *save;
    while (*s && is_delim(*s, delim)) s++;    /* skip leading delimiters */
    if (!*s) { *save = s; return NULL; }
    tok = s;
    while (*s && !is_delim(*s, delim)) s++;
    if (*s) *s++ = '\0';
    *save = s;
    return tok;
}

char *my_strtok(char *s, const char *delim)
{
    static char *save;                         /* the hidden state */
    return my_strtok_r(s, delim, &save);
}

int main(void)
{
    char cmd[] = "  SET,LED,,1  ";
    char csv[] = "a=1;b=2;c=3";
    char *t, *outer, *inner;

    printf("tokens of \"  SET,LED,,1  \":");
    for (t = my_strtok(cmd, ", "); t; t = my_strtok(NULL, ", "))
        printf(" [%s]", t);
    printf("\n");

    /* nested parsing - only possible with the reentrant version */
    for (t = my_strtok_r(csv, ";", &outer); t; t = my_strtok_r(NULL, ";", &outer)) {
        char *k = my_strtok_r(t, "=", &inner);
        char *v = my_strtok_r(NULL, "=", &inner);
        printf("key=%s value=%s\n", k, v);
    }
    return 0;
}
