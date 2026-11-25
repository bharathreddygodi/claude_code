/*
 * DRV-04: I2C master driver - register read/write against a simulated sensor
 * Bus facts: 2 open-drain wires (SDA, SCL) + pull-up resistors, 7-bit address,
 *   START (SDA falls while SCL high), STOP (SDA rises while SCL high),
 *   every byte is followed by an ACK (0) / NACK (1) from the receiver,
 *   repeated START (Sr) switches direction without releasing the bus.
 * Write reg : S [addr|W] A [reg] A [data] A P
 * Read reg  : S [addr|W] A [reg] A Sr [addr|R] A [data] NA P   (master NACKs the last byte)
 * Errors to handle: NACK on address (device absent), bus stuck low (clock out 9 pulses),
 * clock stretching timeout. This program prints the frames as they'd appear on a logic analyser.
 */
#include <stdio.h>
#include <stdint.h>

#define SENSOR_ADDR 0x48

/* --- simulated slave with a register file --- */
static uint8_t regs[16] = {[0x0F] = 0xA1};      /* WHO_AM_I = 0xA1 */
static uint8_t reg_ptr;
static int addressed, rw, byte_idx;
static char trace[256];
static int tlen;
static void tr(const char *s) { while (*s) trace[tlen++] = *s++; trace[tlen] = 0; }

static void bus_start(void) { tr(byte_idx == -1 ? "Sr " : "S "); byte_idx = 0; addressed = 0; }
static void bus_stop(void)  { tr("P"); byte_idx = -2; }

static int bus_write(uint8_t b)                  /* returns 0 = ACK, 1 = NACK */
{
    char s[16];
    int ack;
    if (byte_idx == 0) {
        addressed = ((b >> 1) == SENSOR_ADDR);
        rw = b & 1;
        ack = addressed ? 0 : 1;
        snprintf(s, sizeof s, "[%02X|%c] %s ", b >> 1, rw ? 'R' : 'W', ack ? "NA" : "A");
    } else {
        if (byte_idx == 1) reg_ptr = b & 0x0F;    /* first data byte = register address */
        else regs[reg_ptr++ & 0x0F] = b;          /* auto-increment */
        ack = 0;
        snprintf(s, sizeof s, "[%02X] A ", b);
    }
    byte_idx++;
    tr(s);
    return ack;
}

static uint8_t bus_read(int send_ack)
{
    char s[16];
    uint8_t v = regs[reg_ptr++ & 0x0F];
    snprintf(s, sizeof s, "[%02X] %s ", v, send_ack ? "A" : "NA");
    tr(s);
    return v;
}

/* --- master driver API --- */
int i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, int n)
{
    int i;
    tlen = 0; byte_idx = -2;
    bus_start();
    if (bus_write((uint8_t)(addr << 1))) { bus_stop(); return -1; }   /* device absent */
    bus_write(reg);
    for (i = 0; i < n; i++) bus_write(data[i]);
    bus_stop();
    return 0;
}

int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, int n)
{
    int i;
    tlen = 0; byte_idx = -2;
    bus_start();
    if (bus_write((uint8_t)(addr << 1))) { bus_stop(); return -1; }
    bus_write(reg);
    byte_idx = -1;                                /* repeated start */
    bus_start();
    bus_write((uint8_t)(addr << 1 | 1));
    for (i = 0; i < n; i++) data[i] = bus_read(i < n - 1);   /* NACK the last byte */
    bus_stop();
    return 0;
}

int main(void)
{
    uint8_t buf[3], cfg[2] = {0x60, 0x0A};
    int rc;

    rc = i2c_read_reg(SENSOR_ADDR, 0x0F, buf, 1);
    printf("read WHO_AM_I : rc=%d val=0x%02X\n  bus: %s\n", rc, buf[0], trace);

    rc = i2c_write_reg(SENSOR_ADDR, 0x01, cfg, 2);
    printf("write 2 bytes : rc=%d\n  bus: %s\n", rc, trace);

    rc = i2c_read_reg(SENSOR_ADDR, 0x01, buf, 2);
    printf("read back     : rc=%d 0x%02X 0x%02X\n  bus: %s\n", rc, buf[0], buf[1], trace);

    rc = i2c_read_reg(0x50, 0x00, buf, 1);
    printf("wrong address : rc=%d (NACK -> device not present)\n  bus: %s\n", rc, trace);
    return 0;
}
