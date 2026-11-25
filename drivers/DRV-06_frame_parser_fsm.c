/*
 * DRV-06: Byte-by-byte packet parser state machine (UART / RS-485 protocols)
 * Frame:  0xAA | LEN | CMD | PAYLOAD[LEN] | CHK     CHK = XOR of LEN, CMD, PAYLOAD
 * Feed one byte at a time (from the RX ring buffer); never assume a whole frame arrives.
 * Robustness: resync on garbage, reject LEN > max, bad checksum, and add an
 * inter-byte timeout in real code to drop half-received frames.
 */
#include <stdio.h>
#include <stdint.h>

#define SOF 0xAA
#define MAX_PAYLOAD 16

typedef enum { WAIT_SOF, GET_LEN, GET_CMD, GET_PAYLOAD, GET_CHK } pstate_t;

typedef struct {
    pstate_t st;
    uint8_t len, cmd, idx, chk;
    uint8_t payload[MAX_PAYLOAD];
    unsigned ok, bad_chk, bad_len;
} parser_t;

static void on_frame(const parser_t *p)
{
    int i;
    printf("  FRAME cmd=0x%02X len=%u payload:", p->cmd, p->len);
    for (i = 0; i < p->len; i++) printf(" %02X", p->payload[i]);
    printf("\n");
}

void parser_feed(parser_t *p, uint8_t b)
{
    switch (p->st) {
    case WAIT_SOF:
        if (b == SOF) p->st = GET_LEN;            /* anything else = garbage, skip */
        break;
    case GET_LEN:
        if (b > MAX_PAYLOAD) { p->bad_len++; p->st = WAIT_SOF; break; }
        p->len = b; p->chk = b; p->st = GET_CMD;
        break;
    case GET_CMD:
        p->cmd = b; p->chk ^= b; p->idx = 0;
        p->st = p->len ? GET_PAYLOAD : GET_CHK;
        break;
    case GET_PAYLOAD:
        p->payload[p->idx++] = b; p->chk ^= b;
        if (p->idx == p->len) p->st = GET_CHK;
        break;
    case GET_CHK:
        if (b == p->chk) { p->ok++; on_frame(p); }
        else { p->bad_chk++; printf("  checksum error (got %02X want %02X)\n", b, p->chk); }
        p->st = WAIT_SOF;
        break;
    }
}

int main(void)
{
    uint8_t stream[] = {
        0x00, 0x13,                               /* line noise */
        0xAA, 0x02, 0x10, 0x01, 0x02, 0x11,       /* good: chk = 02^10^01^02 = 0x11 */
        0xAA, 0x01, 0x20, 0x05, 0x99,             /* bad checksum */
        0xAA, 0x40,                               /* bad length */
        0xAA, 0x00, 0x30, 0x30,                   /* good, empty payload */
    };
    parser_t p = {0};
    unsigned i;
    for (i = 0; i < sizeof stream; i++) parser_feed(&p, stream[i]);
    printf("ok=%u bad_chk=%u bad_len=%u\n", p.ok, p.bad_chk, p.bad_len);
    return 0;
}
