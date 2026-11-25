/*
 * BIT-11: Hamming distance
 *   distance(a, b) = popcount(a ^ b)      (XOR marks the bits that differ)
 * Total Hamming distance over all pairs of an array:
 *   naive O(n^2 * 32). Better O(32 * n): for each bit position count how many numbers
 *   have it set (k); that bit contributes k * (n - k) to the total (every 1 paired with every 0).
 * Embedded angle: counting bit errors between sent and received data, comparing
 * ECC codewords, choosing state encodings that differ by few bits.
 */
#include <stdio.h>
#include <stdint.h>

int hamming(uint32_t a, uint32_t b)
{
    uint32_t x = a ^ b;
    int c = 0;
    while (x) { x &= x - 1; c++; }
    return c;
}

long total_hamming_naive(const uint32_t *v, int n)
{
    long t = 0;
    int i, j;
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++) t += hamming(v[i], v[j]);
    return t;
}

long total_hamming_fast(const uint32_t *v, int n)
{
    long t = 0;
    int bit, i;
    for (bit = 0; bit < 32; bit++) {
        long k = 0;
        for (i = 0; i < n; i++) k += (v[i] >> bit) & 1u;
        t += k * (n - k);
    }
    return t;
}

int main(void)
{
    uint32_t v[] = {4, 14, 2, 0xFFFF0000u, 7};
    uint8_t sent[] = {0x55, 0xAA, 0x0F}, recv[] = {0x57, 0xAA, 0x8F};
    int errors = 0;
    unsigned i;

    printf("hamming(1, 4)   = %d (0001 vs 0100 -> expect 2)\n", hamming(1, 4));
    printf("hamming(0, ~0u) = %d\n", hamming(0, ~0u));
    printf("total naive = %ld, total fast = %ld -> %s\n", total_hamming_naive(v, 5), total_hamming_fast(v, 5),
           total_hamming_naive(v, 5) == total_hamming_fast(v, 5) ? "PASS" : "FAIL");

    for (i = 0; i < sizeof sent; i++) errors += hamming(sent[i], recv[i]);
    printf("bit errors between sent and received bytes = %d\n", errors);
    return 0;
}
