// Remove K Digits
// Prompt: Given a string num representing a non-negative integer and an int k,
//         remove k digits so the resulting number is the smallest possible.
//
// Idea: greedy + monotonic increasing stack.
//   A number is smaller when its leftmost digits are smaller. So scanning left
//   to right, whenever the current digit is smaller than the digit before it,
//   removing that earlier (bigger) digit makes the number smaller.
//   - For each digit c: while k > 0 and stack top > c, pop (uses up one k).
//     Then push c.
//   - If k is still > 0, the stack is increasing, so remove from the end.
//   - Strip leading zeros; if nothing is left, the answer is "0".
// Example: num = "1432219", k = 3 -> remove 4, 3, 2 -> "1219".
// Time: O(n), each digit is pushed and popped at most once. Space: O(n).

#include <iostream>
#include <string>
#include <cassert>
using namespace std;

string removeKdigits(string num, int k) {
    string st;                              // string used as the stack
    for (char c : num) {
        while (k > 0 && !st.empty() && st.back() > c) {
            st.pop_back();
            k--;
        }
        st.push_back(c);
    }
    while (k > 0 && !st.empty()) { st.pop_back(); k--; }

    int i = 0;
    while (i < (int)st.size() && st[i] == '0') i++;   // skip leading zeros
    string res = st.substr(i);
    return res.empty() ? "0" : res;
}

int main() {
    assert(removeKdigits("1432219", 3) == "1219");
    assert(removeKdigits("10200", 1) == "200");     // leading zeros stripped
    assert(removeKdigits("10", 2) == "0");          // everything removed
    assert(removeKdigits("12345", 2) == "123");     // increasing: drop from end
    assert(removeKdigits("54321", 2) == "321");     // decreasing: drop from front
    assert(removeKdigits("112", 1) == "11");        // duplicates
    assert(removeKdigits("9", 0) == "9");           // k = 0

    cout << "All tests passed\n";
    return 0;
}
