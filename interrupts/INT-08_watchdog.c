/*
 * INT-08: Watchdog timer with per-task health check-in
 * A hardware watchdog resets the MCU if it isn't "kicked" before it counts down.
 * WRONG: kicking it from a timer ISR - the ISR keeps running even if tasks are hung.
 * RIGHT: every task sets its bit in a check-in mask; a supervisor kicks the watchdog
 *        only when ALL bits are set, then clears the mask.
 * Also: window watchdog (kicking too EARLY is also a fault), read the reset-cause
 * register at boot to log watchdog resets.
 */
#include <stdio.h>
#include <stdint.h>

#define WDT_TIMEOUT 5               /* supervisor periods */
#define TASK_SENSOR (1u << 0)
#define TASK_COMMS  (1u << 1)
#define TASK_CTRL   (1u << 2)
#define ALL_TASKS   (TASK_SENSOR | TASK_COMMS | TASK_CTRL)

static volatile uint32_t checkin;
static int wdt_counter = WDT_TIMEOUT;

static void wdt_kick(void) { wdt_counter = WDT_TIMEOUT; }
static int  wdt_tick(void) { return --wdt_counter <= 0; }    /* 1 = reset fires */

static void task_checkin(uint32_t bit) { checkin |= bit; }   /* atomic OR on real HW */

static void supervisor(int period)
{
    if ((checkin & ALL_TASKS) == ALL_TASKS) {
        wdt_kick();
        checkin = 0;
        printf("period %2d: all tasks alive -> kick\n", period);
    } else {
        printf("period %2d: missing check-in mask 0x%X -> NO kick (wdt=%d)\n",
               period, (unsigned)(ALL_TASKS & ~checkin), wdt_counter - 1);
    }
}

int main(void)
{
    int period;
    for (period = 0; period < 20; period++) {
        task_checkin(TASK_SENSOR);
        if (period < 6) task_checkin(TASK_COMMS);   /* comms task hangs at period 6 */
        task_checkin(TASK_CTRL);
        supervisor(period);
        if (wdt_tick()) {
            printf("*** WATCHDOG RESET at period %d (comms task hung) ***\n", period);
            break;
        }
    }
    return 0;
}
