#pragma once

#ifndef NESPAD_H
#define NESPAD_H

#include <hardware/pio.h>

#define NES_CLOCK 0
#define NES_LATCH 1
#define NES_DATA 2
#define NES_POWER 3

// Button mappings
#define DPAD_LEFT   0x001000
#define DPAD_RIGHT  0x004000
#define DPAD_DOWN   0x000400
#define DPAD_UP     0x000100
#define DPAD_START  0x000040
#define DPAD_SELECT 0x000010
#define DPAD_B      0x000004   // Y on SNES
#define DPAD_A      0x000001   // B on SNES

#define DPAD_Y      0x010000   // A on SNES
#define DPAD_X      0x040000
#define DPAD_LT     0x100000
#define DPAD_RT     0x400000

// PIO program defines
#define NESPAD_WRAP_TARGET 0
#define NESPAD_WRAP 6

// PIO program instructions
static const uint16_t nespad_program_instructions[] = {
    //     .wrap_target
    0x80a0, //  0: pull   block
    0xea01, //  1: set    pins, 1         side 0 [10]
    0xe02f, //  2: set    x, 15           side 0
    0xe000, //  3: set    pins, 0         side 0
    0x4402, //  4: in     pins, 2         side 0 [4]      <--- 2
    0xf500, //  5: set    pins, 0         side 1 [5]
    0x0044, //  6: jmp    x--, 4          side 0
            //     .wrap
};

// PIO program structure
static const struct pio_program nespad_program = {
    .instructions = nespad_program_instructions,
    .length = 7,
    .origin = -1,
};

// Global state variables
static PIO pio = pio1;
static int sm = -1;
static volatile uint32_t nespad_state = 0;   // Joystick 1
static volatile  uint32_t nespad_state2 = 0;  // Joystick 2

/**
 * Get default PIO state machine configuration
 */
static inline pio_sm_config nespad_program_get_default_config(const uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + NESPAD_WRAP_TARGET, offset + NESPAD_WRAP);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

/**
 * Initialize the NES/SNES controller interface
 *
 * @param cpu_khz CPU clock rate in kHz
 * @param clkPin Clock pin number
 * @param dataPin Data pin number
 * @param latPin Latch pin number
 * @return true if initialization successful, false otherwise
 */
static inline bool nes_init() {
    if (pio_can_add_program(pio, &nespad_program) &&
        (sm = pio_claim_unused_sm(pio, true)) >= 0) {
        gpio_init(NES_POWER);
        gpio_set_dir(NES_POWER, GPIO_OUT);
        gpio_put(NES_POWER, 1);

        const uint offset = pio_add_program(pio, &nespad_program);
        pio_sm_config c = nespad_program_get_default_config(offset);

        sm_config_set_sideset_pins(&c, NES_CLOCK);
        sm_config_set_in_pins(&c, NES_DATA);
        sm_config_set_set_pins(&c, NES_LATCH, 1);
        pio_gpio_init(pio, NES_CLOCK);
        pio_gpio_init(pio, NES_DATA);
        // pio_gpio_init(pio, dataPin+1);  // +1 Pin for Joystick2
        pio_gpio_init(pio, NES_LATCH);
        gpio_set_pulls(NES_DATA, true, false); // Pull data high, 0xFF if unplugged
        // gpio_set_pulls(dataPin+1, true, false); // Pull data high, 0xFF if unplugged for Joystick2

        pio_sm_set_pindirs_with_mask(pio, sm,
                                    (1 << NES_CLOCK) | (1 << NES_LATCH), // Outputs
                                    (1 << NES_CLOCK) | (1 << NES_LATCH) |
                                    (1 << NES_DATA)
                                    // | (1 << (dataPin+1))
                                    ); // All pins
        sm_config_set_in_shift(&c, true, true, 32); // R shift, autopush @ 8 bits (@ 16 bits for 2 Joystick)

        sm_config_set_clkdiv_int_frac(&c, clock_get_hz(clk_sys) / 1000000, 0); // 1 MHz clock

        pio_sm_clear_fifos(pio, sm);

        pio_sm_init(pio, sm, offset, &c);
        pio_sm_set_enabled(pio, sm, true);
        pio->txf[sm] = 0;
        return true; // Success
    }
    return false;
}

/**
 * Read controller state
 * Should be called ~100 µs after triggering a read
 * Updates nespad_state and nespad_state2 global variables
 * Button mapping: 0x80=Right, 0x40=Left, 0x20=Down, 0x10=Up,
 *                 0x08=Start, 0x04=Select, 0x02=B, 0x01=A
 */
static inline void nes_read(void) {
    // Check if PIO is initialized and data is available
    if (sm < 0 || pio_sm_is_rx_fifo_empty(pio, sm)) {
        return;
    }

    // Read data from FIFO and invert (controllers use active-low signals)
    const uint32_t raw_data = pio->rxf[sm] ^ 0xFFFFFFFF;

    // Trigger next read
    pio->txf[sm] = 0;

    // Extract controller data - mask with 0x555555 to get only the odd bits
    // (captures every other bit for controller 1)
    nespad_state = raw_data & 0x555555;

    // Extract controller 2 data - shift right by 1 and mask with 0x555555
    // (captures every other bit for controller 2)
    nespad_state2 = raw_data >> 1 & 0x555555;
}

/**
 * Get controller 1 state
 * @return Current button state for controller 1
 */
static inline uint32_t nespad_get_state(void) {
    return nespad_state;
}

/**
 * Get controller 2 state
 * @return Current button state for controller 2
 */
static inline uint32_t nespad_get_state2(void) {
    return nespad_state2;
}

#endif // NESPAD_H