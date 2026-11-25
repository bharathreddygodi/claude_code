/*
 * DRV-07: Driver abstraction layer - ops table (Linux file_operations style)
 * Application code talks to a generic device interface; each driver fills in a
 * struct of function pointers. Benefits: swap hardware without touching app code,
 * unit-test with a mock driver, multiple instances of the same driver (priv data).
 * Same idea as Zephyr device API, Linux char drivers, HAL layers.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct device;
typedef struct {
    int (*open)(struct device *d);
    int (*read)(struct device *d, void *buf, int n);
    int (*write)(struct device *d, const void *buf, int n);
    int (*ioctl)(struct device *d, int cmd, void *arg);
} dev_ops_t;

typedef struct device {
    const char *name;
    const dev_ops_t *ops;
    void *priv;                 /* per-instance driver state */
} device_t;

/* ---- driver 1: UART (instance state = baud, base address) ---- */
typedef struct { uint32_t base; uint32_t baud; } uart_priv_t;
enum { IOCTL_SET_BAUD = 1, IOCTL_GET_TEMP_C = 2 };

static int uart_open(device_t *d) { uart_priv_t *p = d->priv; printf("  %s open @0x%08X\n", d->name, (unsigned)p->base); return 0; }
static int uart_write(device_t *d, const void *b, int n) { printf("  %s tx: %.*s\n", d->name, n, (const char *)b); return n; }
static int uart_read(device_t *d, void *b, int n) { (void)d; return n > 2 ? (memcpy(b, "OK", 2), 2) : 0; }
static int uart_ioctl(device_t *d, int cmd, void *arg)
{
    uart_priv_t *p = d->priv;
    if (cmd == IOCTL_SET_BAUD) { p->baud = *(uint32_t *)arg; printf("  %s baud=%u\n", d->name, (unsigned)p->baud); return 0; }
    return -1;                  /* -ENOTTY: unsupported command */
}
static const dev_ops_t uart_ops = {uart_open, uart_read, uart_write, uart_ioctl};

/* ---- driver 2: temperature sensor (mock - great for unit tests) ---- */
static int temp_open(device_t *d) { printf("  %s open\n", d->name); return 0; }
static int temp_ioctl(device_t *d, int cmd, void *arg)
{
    (void)d;
    if (cmd == IOCTL_GET_TEMP_C) { *(int *)arg = 23; return 0; }
    return -1;
}
static const dev_ops_t temp_ops = {temp_open, NULL, NULL, temp_ioctl};   /* no read/write */

/* ---- generic layer: NULL-checks so drivers can leave ops unimplemented ---- */
static int dev_write(device_t *d, const void *b, int n) { return d->ops->write ? d->ops->write(d, b, n) : -1; }
static int dev_ioctl(device_t *d, int c, void *a)       { return d->ops->ioctl ? d->ops->ioctl(d, c, a) : -1; }

int main(void)
{
    uart_priv_t u1 = {0x40011000u, 9600}, u2 = {0x40004400u, 9600};
    device_t devs[] = {
        {"uart1", &uart_ops, &u1},
        {"uart2", &uart_ops, &u2},          /* same driver, second instance */
        {"temp0", &temp_ops, NULL},
    };
    uint32_t baud = 115200;
    int t = 0;
    char rx[8];
    unsigned i;

    for (i = 0; i < 3; i++) devs[i].ops->open(&devs[i]);
    dev_write(&devs[0], "hello", 5);
    dev_ioctl(&devs[1], IOCTL_SET_BAUD, &baud);
    printf("  uart1 read %d bytes\n", devs[0].ops->read(&devs[0], rx, sizeof rx));
    printf("  temp0 write -> %d (not supported)\n", dev_write(&devs[2], "x", 1));
    dev_ioctl(&devs[2], IOCTL_GET_TEMP_C, &t);
    printf("  temp0 = %d C | uart1 baud=%u uart2 baud=%u\n", t, (unsigned)u1.baud, (unsigned)u2.baud);
    return 0;
}
