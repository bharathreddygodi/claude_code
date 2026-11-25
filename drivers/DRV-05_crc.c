/*
 * DRV-05: CRC - bitwise and table-driven (CRC-8, CRC-16/CCITT-FALSE, CRC-32)
 * CRC = remainder of polynomial division over GF(2) = shift + XOR.
 *   bitwise: 8 iterations per byte, no table (tiny flash)
 *   table  : 256-entry table, one lookup per byte (8x faster, 256/512/1024 bytes)
 * Know the parameters: width, poly, init, reflect in/out, xorout. Check values below
 * are the standard ones for the string "123456789".
 * Checksum vs CRC: a simple sum misses swapped bytes; CRC catches all burst errors <= width.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint8_t crc8(const uint8_t *d, size_t n)           /* poly 0x07, init 0 (CRC-8/SMBUS) */
{
    uint8_t crc = 0;
    while (n--) {
        int i;
        crc ^= *d++;
        for (i = 0; i < 8; i++) crc = (uint8_t)((crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1);
    }
    return crc;
}

uint16_t crc16_ccitt(const uint8_t *d, size_t n)   /* poly 0x1021, init 0xFFFF */
{
    uint16_t crc = 0xFFFF;
    while (n--) {
        int i;
        crc ^= (uint16_t)(*d++ << 8);
        for (i = 0; i < 8; i++) crc = (uint16_t)((crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1);
    }
    return crc;
}

uint32_t crc32_bitwise(const uint8_t *d, size_t n) /* reflected poly 0xEDB88320 (Ethernet/zip) */
{
    uint32_t crc = 0xFFFFFFFFu;
    while (n--) {
        int i;
        crc ^= *d++;
        for (i = 0; i < 8; i++) crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
    }
    return ~crc;
}

static uint32_t crc32_table[256];
static void crc32_init(void)
{
    uint32_t i, c;
    int k;
    for (i = 0; i < 256; i++) {
        c = i;
        for (k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1;
        crc32_table[i] = c;
    }
}
uint32_t crc32_table_fn(const uint8_t *d, size_t n)
{
    uint32_t crc = 0xFFFFFFFFu;
    while (n--) crc = (crc >> 8) ^ crc32_table[(crc ^ *d++) & 0xFF];
    return ~crc;
}

int main(void)
{
    const uint8_t *s = (const uint8_t *)"123456789";
    uint8_t pkt[] = {0x01, 0x02, 0x03, 0x04};
    uint32_t good;

    crc32_init();
    printf("CRC-8/SMBUS      = 0x%02X   (expect 0xF4)\n", crc8(s, 9));
    printf("CRC-16/CCITT     = 0x%04X (expect 0x29B1)\n", crc16_ccitt(s, 9));
    printf("CRC-32 bitwise   = 0x%08X (expect 0xCBF43926)\n", (unsigned)crc32_bitwise(s, 9));
    printf("CRC-32 table     = 0x%08X (expect 0xCBF43926)\n", (unsigned)crc32_table_fn(s, 9));

    good = crc32_table_fn(pkt, 4);
    pkt[2] ^= 0x10;                                   /* flip one bit "in transit" */
    printf("1-bit error detected: %s\n", crc32_table_fn(pkt, 4) != good ? "YES" : "no");
    return 0;
}
