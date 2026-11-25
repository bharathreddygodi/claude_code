/*
 * TMR-24: Task Scheduler (LeetCode 621)
 * Tasks are letters; the same letter must be at least n slots apart. Minimum slots?
 * Formula (O(N)): let maxf = highest frequency, cnt = how many letters have it.
 *     answer = max(len(tasks), (maxf - 1) * (n + 1) + cnt)
 *   The most frequent letter forms (maxf - 1) frames of size n+1, plus a final partial
 *   frame holding every letter that ties for max. If there are enough other tasks to
 *   fill all idle gaps, no idle is needed and the answer is just len(tasks).
 * Greedy simulation (to print the schedule): each slot, run the ready letter with the
 * most remaining copies; if none is ready, idle.
 * Embedded link: same shape as "a peripheral needs a cooldown between commands"
 * (flash erase, radio duty-cycle limits).
 */
#include <stdio.h>
#include <string.h>

int least_interval(const char *tasks, int n)
{
    int f[26] = {0}, maxf = 0, cnt = 0, i, len = (int)strlen(tasks), frames;
    for (i = 0; tasks[i]; i++) f[tasks[i] - 'A']++;
    for (i = 0; i < 26; i++) if (f[i] > maxf) maxf = f[i];
    for (i = 0; i < 26; i++) if (f[i] == maxf) cnt++;
    frames = (maxf - 1) * (n + 1) + cnt;
    return frames > len ? frames : len;
}

int simulate(const char *tasks, int n, char *out, int cap)
{
    int left[26] = {0}, ready_at[26] = {0}, remaining = 0, t = 0, i;
    for (i = 0; tasks[i]; i++) { left[tasks[i] - 'A']++; remaining++; }
    while (remaining && t < cap - 1) {
        int best = -1;
        for (i = 0; i < 26; i++)
            if (left[i] && ready_at[i] <= t && (best < 0 || left[i] > left[best])) best = i;
        if (best < 0) out[t] = '.';
        else { out[t] = (char)('A' + best); left[best]--; ready_at[best] = t + n + 1; remaining--; }
        t++;
    }
    out[t] = '\0';
    return t;
}

int main(void)
{
    struct { const char *tasks; int n, expect; } tc[] = {
        {"AAABBB", 2, 8}, {"AAABBB", 0, 6}, {"AAAAAABCDEFG", 2, 16}, {"AABBCCDD", 2, 8}, {"AAABBBCC", 3, 10},
    };
    unsigned i;
    char sched[64];
    for (i = 0; i < sizeof tc / sizeof tc[0]; i++) {
        int f = least_interval(tc[i].tasks, tc[i].n);
        int s = simulate(tc[i].tasks, tc[i].n, sched, sizeof sched);
        printf("tasks=%-13s n=%d formula=%2d greedy=%2d expect=%2d %s  schedule: %s ('.' = idle)\n",
               tc[i].tasks, tc[i].n, f, s, tc[i].expect, (f == tc[i].expect && s == f) ? "PASS" : "FAIL", sched);
    }
    return 0;
}
