#include <stdio.h>

#include <hardware/gpio.h>
#include <hardware/clocks.h>

#include <pico/time.h>
#include <pico/multicore.h>

#include "ws2812b.h"
#include "nespad.h"
#include "genesispad.h"
#include "pico/stdio_usb.h"

// PC Engine connector pinout (facing console)
/*
1 - 5V
2 - Up / A
3 - Right / B
4 - Down / Select
5 - Left / Start (run)
6 - SELECT
7 - ENABLE
8 - GND

    ___________
   /     7     \
  /  8       6  \
 |               |
 | 5    4      3 |
 |               |
  \  2       1  /
   \___________/
*/

/*
*DB9 to RP2040

DB9 | RP2040
-----+--------
1  | GPIO7
2  | GPIO2
3  | GPIO1
4  | GPI0
5  | GPIO8
6  | GPIO3
7  | GPIO4
8  | GND
9  | GPIO6



RP2040 to PC ENGINE

PCE | RP2040
-----+--------
1  | +5V
2  | GPIO26
3  | GPIO27
4  | GPIO28
5  | GPIO29
6  | GPIO14
7  | GPIO15
8  | GND
-----+--------
*/
#define PCE_SELECT 14
#define PCE_ENABLE 15 // active = low

#define PCE_D0 26 // Pin 2
#define PCE_D1 27 // Pin 3
#define PCE_D2 28 // Pin 4
#define PCE_D3 29 // Pin 5


typedef union {
    uint8_t buttons_raw; // Raw button state for fast processing
    struct {
        uint8_t up: 1;
        uint8_t down: 1;
        uint8_t left: 1;
        uint8_t right: 1;
        uint8_t i: 1;
        uint8_t ii: 1;
        uint8_t iii: 1;
        uint8_t iv: 1;
        uint8_t v: 1;
        uint8_t vi: 1;
        uint8_t start: 1;
        uint8_t select: 1;
    } buttons;
} pcepad_t;

static volatile pcepad_t pcepad;

static void initialize_pcepad() {
    // Inputs
    gpio_init(PCE_SELECT);
    gpio_init(PCE_ENABLE);

    gpio_set_dir(PCE_SELECT, GPIO_IN);
    gpio_set_dir(PCE_ENABLE, GPIO_IN);

    // Outputs
    gpio_init(PCE_D0);
    gpio_init(PCE_D1);
    gpio_init(PCE_D2);
    gpio_init(PCE_D3);

    gpio_set_dir(PCE_D0, GPIO_OUT);
    gpio_set_dir(PCE_D1, GPIO_OUT);
    gpio_set_dir(PCE_D2, GPIO_OUT);
    gpio_set_dir(PCE_D3, GPIO_OUT);

    gpio_put(PCE_D0, 1);
    gpio_put(PCE_D1, 1);
    gpio_put(PCE_D2, 1);
    gpio_put(PCE_D3, 1);

    pcepad.buttons_raw = 0xFF;
}

[[noreturn]] void second_core() {
    init_ws2812b();
    uint8_t brightness = 127;

    // Check if we have Genesis gamepad connected
    if (genesis_init()) {
        while (1) {
            set_ws2812b_HSV(0, genesis_sixbuttons ? WS2812_PURPLE : WS2812_CYAN, 255, brightness--);
            if (brightness == 63) brightness = 127;

            genesis_read();

            pcepad.buttons.up = genesis.buttons.up;
            pcepad.buttons.right = genesis.buttons.right;
            pcepad.buttons.down = genesis.buttons.down;
            pcepad.buttons.left = genesis.buttons.left;

            pcepad.buttons.i = genesis.buttons.b;
            pcepad.buttons.ii = genesis.buttons.a;
            pcepad.buttons.start = genesis.buttons.start;
            pcepad.buttons.select = genesis.buttons.mode & genesis.buttons.c;
            sleep_ms(8);
        }
    }

    // Otherwise it's NES gamepad
    nes_init();

    while (1) {
        set_ws2812b_HSV(0, WS2812_YELLOW, 250, brightness--);
        if (brightness == 63) brightness = 127;

        nes_read();

        pcepad.buttons.up = !(nespad_state & DPAD_UP);
        pcepad.buttons.right = !(nespad_state & DPAD_RIGHT);
        pcepad.buttons.down = !(nespad_state & DPAD_DOWN);
        pcepad.buttons.left = !(nespad_state & DPAD_LEFT);

        pcepad.buttons.i = !(nespad_state & DPAD_A);
        pcepad.buttons.ii = !(nespad_state & DPAD_B);
        pcepad.buttons.start = !(nespad_state & DPAD_START);
        pcepad.buttons.select = !(nespad_state & DPAD_SELECT);

        sleep_ms(8);
    }
    __unreachable();
}

[[noreturn]] void main() {
    set_sys_clock_khz(250 * KHZ, true);

    sleep_ms(33);

    // stdio_usb_init();
    multicore_launch_core1(second_core);

    sleep_ms(33);
    initialize_pcepad();

    uint8_t cycle = 0, last_select = 0, current_controller = 0;

    while (true) {
        const uint8_t enable = gpio_get(PCE_ENABLE);

        if (!enable) {
            const uint8_t select = gpio_get(PCE_SELECT);

            if (current_controller == 0) {
                if (select) {
                    gpio_put(PCE_D0, pcepad.buttons.up);
                    gpio_put(PCE_D1, pcepad.buttons.right);
                    gpio_put(PCE_D2, pcepad.buttons.down);
                    gpio_put(PCE_D3, pcepad.buttons.left);
                } else {
                    gpio_put(PCE_D0, pcepad.buttons.i);
                    gpio_put(PCE_D1, pcepad.buttons.ii);
                    gpio_put(PCE_D2, pcepad.buttons.select);
                    gpio_put(PCE_D3, pcepad.buttons.start);
                }
            } else {
                gpio_put(PCE_D0, 1);
                gpio_put(PCE_D1, 1);
                gpio_put(PCE_D2, 1);
                gpio_put(PCE_D3, 1);
            }

            if (select != last_select) {
                last_select = select;
                cycle++;
                current_controller = cycle >> 1;
            }
        } else {
            cycle = 0;
            current_controller = 0;
        }
    }

    __unreachable();
}
