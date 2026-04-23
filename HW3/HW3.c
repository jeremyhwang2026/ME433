#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// MCP23008 7-bit Address (A0, A1, A2 grounded)
#define ADDR 0x20

// MCP23008 Registers
#define IODIR 0x00
#define GPIO  0x09
#define OLAT  0x0A

// Pico Heartbeat LED
const uint PICO_LED = 25;

void mcp_write(uint8_t reg, uint8_t val) {
    uint8_t buf[] = {reg, val};
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
}

uint8_t mcp_read(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, ADDR, &val, 1, false);
    return val;
}

int main() {
    stdio_init_all();

    // Initialize I2C at 100kHz
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    // Initialize Heartbeat LED
    gpio_init(PICO_LED);
    gpio_set_dir(PICO_LED, GPIO_OUT);

    // 1. CHIP INITIALIZATION
    // Set GP7 as Output (0) and GP0 as Input (1). 
    // Binary: 0111 1111 = 0x7F
    mcp_write(IODIR, 0x7F);

    while (true) {
        // --- Heartbeat ---
        static absolute_time_t next_blink;
        if (get_absolute_time() > next_blink) {
            gpio_put(PICO_LED, !gpio_get(PICO_LED));
            next_blink = make_timeout_time_ms(500);
        }

        // --- Read Button (GP0) ---
        uint8_t current_pins = mcp_read(GPIO);
        
        // Check if bit 0 is low (button pressed)
        // Using a 10k pull-up means 0 = Pressed, 1 = Idle
        if (!(current_pins & 0x01)) {
            // Turn on GP7 (Set bit 7)
            mcp_write(OLAT, 0x80);
        } else {
            // Turn off GP7 (Clear bit 7)
            mcp_write(OLAT, 0x00);
        }
    }
}