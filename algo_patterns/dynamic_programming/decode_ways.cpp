// Decode Ways
// Prompt: Given a string of digits, return the number of ways to decode it,
//         where A = 1, B = 2, ..., Z = 26.
//
// Idea: DP on prefixes. ways(i) = number of ways to decode the first i chars.
//   The last letter used either one digit or two digits:
//   - one digit s[i-1]: valid if it's '1'..'9'  -> add ways(i-1)
//   - two digits s[i-2..i-1]: valid if 10..26  -> add ways(i-2)
//   ways(0) = 1 (empty prefix: one way, decode nothing).
//   Only the last two values are needed, so keep two variables.
//   '0' can't stand alone; it only works as the second digit of 10 or 20.
// Example: "226" -> "2 2 6", "22 6", "2 26" -> 3.
// Time: O(n). Space: O(1).

#include <iostream>
#include <string>
#include <cassert>
using namespace std;

int numDecodings(const string& s) {
    if (s.empty()) return 0;
    int prev2 = 0, prev1 = 1;              // ways(i-2), ways(i-1); ways(0) = 1
    for (int i = 1; i <= (int)s.size(); i++) {
        int cur = 0;
        if (s[i - 1] != '0') cur += prev1;                     // one digit
        if (i >= 2) {
            int two = (s[i - 2] - '0') * 10 + (s[i - 1] - '0');
            if (two >= 10 && two <= 26) cur += prev2;          // two digits
        }
        prev2 = prev1;
        prev1 = cur;
    }
    return prev1;
}

int main() {
    assert(numDecodings("12") == 2);       // "AB", "L"
    assert(numDecodings("226") == 3);
    assert(numDecodings("11106") == 2);    // 1 1 10 6, 11 10 6
    assert(numDecodings("10") == 1);       // only "J"
    assert(numDecodings("27") == 1);       // 27 > 26, only "BG"
    assert(numDecodings("0") == 0);        // leading zero
    assert(numDecodings("06") == 0);       // "06" isn't a valid code
    assert(numDecodings("100") == 0);      // second 0 can't attach to anything
    assert(numDecodings("2101") == 1);     // 2 10 1
    assert(numDecodings("") == 0);

    cout << "All tests passed\n";
    return 0;
}
