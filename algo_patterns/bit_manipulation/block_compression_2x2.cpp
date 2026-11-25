// 2x2 Block Compression of a Grayscale Image
// Prompt: A 4096 x 4096 image of 8-bit pixels. Split it into 2x2 blocks and
//         encode each block as exactly 2 bytes:
//           byte 1 = average of the 4 pixels, rounded (0..255)
//           byte 2 = four 2-bit codes, one per pixel (TL, TR, BL, BR), each
//                    picking the nearest of 4 levels {avg-2d, avg-d, avg+d, avg+2d}
//         16 MB in -> 2048 x 2048 x 2 = 8 MB out (2:1).
//         Follow-up: decoder that rebuilds pixels from avg + code, clamped 0..255.
//
// Design choices (the prompt leaves these open, so state them in the interview):
//   - d is one fixed constant (D), shared by encoder and decoder. A per-block d
//     wouldn't fit in 2 bytes.
//   - Rounding: avg = (sum + 2) / 4, i.e. round half up. Max is (1020+2)/4 = 255.
//   - Code c -> offset OFF[c]: 0 = -2d, 1 = -d, 2 = +d, 3 = +2d.
//   - Packing: TL in bits 7-6, TR in 5-4, BL in 3-2, BR in 1-0.
//   - The encoder compares against the CLAMPED levels, the same values the
//     decoder will produce. Near 0 or 255 this picks the code that actually
//     decodes closest.
//   - Output blocks are stored row-major: block (bx, by) at offset
//     2 * (by * blocksPerRow + bx).
//
// Example (D = 4): block [100, 104, 96, 120] -> avg = 420/4 = 105.
//   Levels: 97, 101, 109, 113.
//   100 -> 101 (code 1), 104 -> 101 (code 1), 96 -> 97 (code 0), 120 -> 113 (code 3)
//   byte 2 = 01 01 00 11 = 0x53. Decodes to [101, 101, 97, 113].
//
// Note: there's no "0" level, so even a perfectly flat block decodes with an
//   error of d. That's a property of the given level set.
// Time: O(W * H), one pass. Space: O(1) besides the input/output buffers.

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cassert>
using namespace std;

const int D = 4;
const int OFF[4] = {-2 * D, -D, D, 2 * D};

int clamp255(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }

// img: w*h bytes, row-major, w and h even. out: (w/2)*(h/2)*2 bytes.
void encode(const uint8_t* img, int w, int h, uint8_t* out) {
    for (int by = 0; by < h; by += 2) {
        for (int bx = 0; bx < w; bx += 2) {
            int p[4];
            for (int i = 0; i < 4; i++)                       // i: 0=TL 1=TR 2=BL 3=BR
                p[i] = img[(by + i / 2) * w + (bx + i % 2)];

            int avg = (p[0] + p[1] + p[2] + p[3] + 2) / 4;

            uint8_t codes = 0;
            for (int i = 0; i < 4; i++) {
                int best = 0;
                for (int c = 1; c < 4; c++)
                    if (abs(p[i] - clamp255(avg + OFF[c])) < abs(p[i] - clamp255(avg + OFF[best])))
                        best = c;
                codes |= best << (6 - 2 * i);
            }
            *out++ = (uint8_t)avg;
            *out++ = codes;
        }
    }
}

// in: (w/2)*(h/2)*2 bytes from encode(). img: w*h bytes.
void decode(const uint8_t* in, int w, int h, uint8_t* img) {
    for (int by = 0; by < h; by += 2) {
        for (int bx = 0; bx < w; bx += 2) {
            int avg = *in++;
            uint8_t codes = *in++;
            for (int i = 0; i < 4; i++) {
                int c = (codes >> (6 - 2 * i)) & 3;
                img[(by + i / 2) * w + (bx + i % 2)] = (uint8_t)clamp255(avg + OFF[c]);
            }
        }
    }
}

int main() {
    // 1. the example block, as a 2x2 image
    {
        uint8_t img[4] = {100, 104, 96, 120};
        uint8_t enc[2], dec[4];
        encode(img, 2, 2, enc);
        assert(enc[0] == 105 && enc[1] == 0x53);
        decode(enc, 2, 2, dec);
        assert(dec[0] == 101 && dec[1] == 101 && dec[2] == 97 && dec[3] == 113);
    }

    // 2. clamping at the edges: all-black and all-white blocks decode exactly
    {
        uint8_t img[8] = {0, 0, 255, 255,
                          0, 0, 255, 255};
        uint8_t enc[4], dec[8];
        encode(img, 4, 2, enc);
        assert(enc[0] == 0 && enc[2] == 255);
        decode(enc, 4, 2, dec);
        for (int i = 0; i < 8; i++) assert(dec[i] == img[i]);
    }

    // 3. rounding: sum 2 -> avg 1 (0.5 rounds up), sum 1 -> avg 0
    {
        uint8_t a[4] = {0, 0, 1, 1}, b[4] = {0, 0, 0, 1}, enc[2];
        encode(a, 2, 2, enc); assert(enc[0] == 1);
        encode(b, 2, 2, enc); assert(enc[0] == 0);
    }

    // 4. full size: 4096 x 4096 smooth image -> 8 MB, every pixel within D
    {
        const int W = 4096, H = 4096;
        vector<uint8_t> img(W * H), enc((W / 2) * (H / 2) * 2), dec(W * H);
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                img[y * W + x] = (uint8_t)((x / 16 + y / 16) / 2);   // 0..255 gradient
        encode(img.data(), W, H, enc.data());
        decode(enc.data(), W, H, dec.data());
        assert(enc.size() == 8u * 1024 * 1024);
        for (int i = 0; i < W * H; i++) assert(abs(dec[i] - img[i]) <= D);
    }

    cout << "All tests passed\n";
    return 0;
}
