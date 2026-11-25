/*
 * DRV-03: SPI - bit-banged master, all 4 modes, against a simulated slave
 * SPI = full duplex: each clock shifts one bit OUT on MOSI and one bit IN on MISO.
 *   CPOL: idle clock level (0 low, 1 high)
 *   CPHA: 0 = sample on the FIRST (leading) edge, 1 = sample on the SECOND (trailing) edge
 *   Mode 0 (CPOL0,CPHA0) is the most common. MSB first usually. CS is active low.
 * Interview: SPI vs I2C vs UART - SPI fastest, no addressing (one CS per slave),
 * no ACK, 4 wires; I2C 2 wires, addressed, ACK, slower; UART async, no clock.
 */
#include <stdio.h>
#include <stdint.h>

static int SCK, MOSI, MISO, CS = 1;
static int cpol, cpha;

/* --- simulated slave: echoes previous byte, shifts on the opposite edge --- */
static uint8_t slave_tx = 0xA5, slave_rx, slave_bits;
static int prev_sck;
static void slave_on_edge(void)
{
    int leading = (SCK != cpol);                 /* edge away from idle level */
    int sample_edge = cpha ? !leading : leading;
    if (CS) return;
    if (SCK == prev_sck) return;
    prev_sck = SCK;
    if (sample_edge) {
        slave_rx = (uint8_t)((slave_rx << 1) | MOSI);
        if (++slave_bits == 8) slave_bits = 0;
    } else {
        MISO = (slave_tx >> 7) & 1;              /* present next bit */
        slave_tx <<= 1;
    }
}
static void set_sck(int v) { SCK = v; slave_on_edge(); }

static void spi_begin(int mode)
{
    cpol = (mode >> 1) & 1; cpha = mode & 1;
    SCK = prev_sck = cpol;
    CS = 0;                                      /* select slave */
    if (!cpha) { MISO = (slave_tx >> 7) & 1; slave_tx <<= 1; }  /* CPHA0: data valid before 1st edge */
}
static void spi_end(void) { CS = 1; }

uint8_t spi_transfer(uint8_t out)
{
    uint8_t in = 0;
    int i;
    for (i = 7; i >= 0; i--) {
        if (!cpha) {                             /* setup, then sample on leading edge */
            MOSI = (out >> i) & 1;
            set_sck(!cpol);  in = (uint8_t)((in << 1) | MISO);
            set_sck(cpol);
        } else {                                 /* shift on leading, sample on trailing */
            set_sck(!cpol);  MOSI = (out >> i) & 1;
            set_sck(cpol);   in = (uint8_t)((in << 1) | MISO);
        }
    }
    return in;
}

int main(void)
{
    int mode;
    for (mode = 0; mode < 4; mode++) {
        uint8_t got;
        slave_tx = 0xA5; slave_rx = 0; slave_bits = 0;
        spi_begin(mode);
        if (cpha) MISO = 0;
        got = spi_transfer(0x3C);
        spi_end();
        printf("mode %d (CPOL=%d CPHA=%d): master sent 0x3C, slave got 0x%02X | slave sent 0xA5, master got 0x%02X %s\n",
               mode, cpol, cpha, slave_rx, got, (slave_rx == 0x3C && got == 0xA5) ? "PASS" : "FAIL");
    }
    return 0;
}
