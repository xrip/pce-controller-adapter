#pragma once
#ifndef GENESIS_PAD_H
#define GENESIS_PAD_H

#include <hardware/gpio.h>

#define GENESIS_POWER 8
#define GENESIS_SELECT 4

#define GENESIS_UP_Z 7
#define GENESIS_DOWN_Y 2
#define GENESIS_LEFT_X 1
#define GENESIS_RIGHT_MODE 0

#define GENESIS_B_A 3
#define GENESIS_C_START 6


static const uint8_t genesis_outputs[] = { GENESIS_POWER, GENESIS_SELECT };
static const uint8_t genesis_inputs[] = { GENESIS_UP_Z, GENESIS_DOWN_Y, GENESIS_LEFT_X, GENESIS_RIGHT_MODE, GENESIS_B_A, GENESIS_C_START };
// Controller state structure
typedef union {
    uint16_t buttons_raw;  // Raw button state for fast processing
    struct {
        uint16_t up    : 1;
        uint16_t down  : 1;
        uint16_t left  : 1;
        uint16_t right : 1;
        uint16_t a     : 1;
        uint16_t b     : 1;
        uint16_t c     : 1;
        uint16_t x     : 1;
        uint16_t y     : 1;
        uint16_t z     : 1;
        uint16_t start : 1;
        uint16_t mode  : 1;
        uint16_t _pad  : 4;  // Padding to 16 bits
    } buttons;
} genesis_state_t;

static volatile genesis_state_t genesis;

static inline bool genesis_init() {
    // Power-on genesis controller and set SELECT to 1
    for (unsigned int i = 0; i < sizeof(genesis_outputs); i++) {
        gpio_init(genesis_outputs[i]);
        gpio_set_dir(genesis_outputs[i], GPIO_OUT);
        gpio_put(genesis_outputs[i], 1);
    }

    busy_wait_ms(1);

    // Initialize inputs
    for (unsigned int i = 0; i < sizeof(genesis_inputs); i++) {
        gpio_init(genesis_inputs[i]);
        gpio_set_dir(genesis_inputs[i], GPIO_IN);
    }

    genesis.buttons_raw = 0xffff;

    return gpio_get(GENESIS_UP_Z) && gpio_get(GENESIS_B_A) && gpio_get(GENESIS_C_START);
}


static inline void genesis_read() {
    gpio_put(GENESIS_SELECT, 1);
    busy_wait_us(20);

    genesis.buttons.up = gpio_get(GENESIS_UP_Z);
    genesis.buttons.down = gpio_get(GENESIS_DOWN_Y);
    genesis.buttons.left = gpio_get(GENESIS_LEFT_X);
    genesis.buttons.right = gpio_get(GENESIS_RIGHT_MODE);

    genesis.buttons.b = gpio_get(GENESIS_B_A);
    genesis.buttons.c = gpio_get(GENESIS_C_START);

    gpio_put(GENESIS_SELECT, 0);
    busy_wait_us(20);

    genesis.buttons.a = gpio_get(GENESIS_B_A);
    genesis.buttons.start = gpio_get(GENESIS_C_START);

    gpio_put(GENESIS_SELECT, 1);
    busy_wait_us(20);
    gpio_put(GENESIS_SELECT, 0);
    busy_wait_us(20);
    gpio_put(GENESIS_SELECT, 1);
    busy_wait_us(20);
    gpio_put(GENESIS_SELECT, 0);
    busy_wait_us(20);
    gpio_put(GENESIS_SELECT, 1);
    busy_wait_us(20);
    gpio_put(GENESIS_SELECT, 0);
    busy_wait_us(20);

    genesis.buttons.x = gpio_get(GENESIS_LEFT_X);
    genesis.buttons.y = gpio_get(GENESIS_DOWN_Y);
    genesis.buttons.z = gpio_get(GENESIS_UP_Z);
/*
    static uint16_t last = 0xffff;
    if (last != genesis.buttons_raw) {
        printf("up: %u, down: %u, left: %u, right: %u, a: %u, b: %u, c: %u, start: %u\n", genesis.buttons.up,  genesis.buttons.down, genesis.buttons.left, genesis.buttons.right, genesis.buttons.a, genesis.buttons.b, genesis.buttons.c, genesis.buttons.start);
        last = genesis.buttons_raw;
    }
    */
}


#endif // GENESIS_PAD_H
