/*
 * DRV-09: ADC processing with integer / fixed-point math (no FPU)
 *  - raw -> millivolts: mv = raw * VREF_MV / 4095  (multiply BEFORE divide, watch overflow)
 *  - Q15/Q16 fixed point: store x * 2^16 in an int; multiply then shift back
 *  - moving average (ring of N samples, running sum: O(1) per sample)
 *  - exponential (IIR) filter: y += (x - y) >> k   (alpha = 1/2^k, one shift, no buffer)
 *  - linear calibration: y = gain * x + offset in Q16
 */
#include <stdio.h>
#include <stdint.h>

#define VREF_MV  3300
#define ADC_MAX  4095

static uint32_t raw_to_mv(uint16_t raw) { return ((uint32_t)raw * VREF_MV + ADC_MAX / 2) / ADC_MAX; } /* rounded */

typedef int32_t q16_t;
#define Q16(x)        ((q16_t)((x) * 65536.0))
#define Q16_MUL(a, b) ((q16_t)(((int64_t)(a) * (b)) >> 16))
#define Q16_TO_INT(a) ((a) >> 16)

#define MA_N 4
typedef struct { uint16_t buf[MA_N]; uint32_t sum; int idx, count; } movavg_t;
static uint16_t movavg(movavg_t *m, uint16_t x)
{
    m->sum -= m->buf[m->idx];
    m->buf[m->idx] = x;
    m->sum += x;
    m->idx = (m->idx + 1) % MA_N;
    if (m->count < MA_N) m->count++;
    return (uint16_t)(m->sum / m->count);
}

typedef struct { int32_t y; int init; } iir_t;           /* y kept with 4 extra fraction bits */
static uint16_t iir(iir_t *f, uint16_t x, int k)
{
    int32_t xs = (int32_t)x << 4;
    if (!f->init) { f->y = xs; f->init = 1; }
    f->y += (xs - f->y) >> k;
    return (uint16_t)((f->y + 8) >> 4);
}

int main(void)
{
    uint16_t samples[] = {2000, 2010, 1990, 3500 /* spike */, 2005, 1995, 2000, 2002};
    movavg_t ma = {{0}, 0, 0, 0};
    iir_t f = {0, 0};
    q16_t gain = Q16(1.02), offset = Q16(-15.0);          /* calibration from production test */
    unsigned i;

    printf("raw 4095 -> %u mV, raw 2048 -> %u mV\n", (unsigned)raw_to_mv(4095), (unsigned)raw_to_mv(2048));
    printf(" raw   mavg  iir(k=2)  calibrated\n");
    for (i = 0; i < sizeof samples / sizeof samples[0]; i++) {
        q16_t cal = Q16_MUL(gain, (q16_t)samples[i] << 16) + offset;
        printf("%5u %6u %8u %11d\n", samples[i], movavg(&ma, samples[i]), iir(&f, samples[i], 2), (int)Q16_TO_INT(cal));
    }
    printf("note: 4095 * 3300 fits in 32 bits; with 16-bit ints it would overflow -> cast first.\n");
    return 0;
}
