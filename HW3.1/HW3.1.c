#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

// MCP23008 7-bit Address (A0, A1, A2 grounded)
#define MCP_ADDR 0x20

// MCP23008 Register Map
#define IODIR 0x00
#define GPIO_REG 0x09
#define OLAT  0x0A

// Pico Heartbeat LED
const uint PICO_LED = 17;

void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char reg);

int main() {
    stdio_init_all();

    // I2C Initialisation at 400Khz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // Initialize Heartbeat LED
    gpio_init(PICO_LED);
    gpio_set_dir(PICO_LED, GPIO_OUT);

    // --- Chip Initialization ---
    // IODIR: 0 = Output, 1 = Input. 
    // We want GP7 (bit 7) as Output, GP0 (bit 0) as Input. 
    // 0111 1111 = 0x7F
    setPin(MCP_ADDR, IODIR, 0x7F);

    // Variable for non-blocking heartbeat timer
    absolute_time_t next_heartbeat = get_absolute_time();
    bool heart_state = false;

    while (true) {
        // 1. Heartbeat Logic (Blinks every 500ms)
        if (absolute_time_diff_us(get_absolute_time(), next_heartbeat) < 0) {
            heart_state = !heart_state;
            gpio_put(PICO_LED, heart_state);
            next_heartbeat = make_timeout_time_ms(500);
        }

        // 2. Read Button from GP0
        unsigned char pins = readPin(MCP_ADDR, GPIO_REG);
        
        // Since GP0 has a 10k pull-up, it is HIGH (1) normally
        // and LOW (0) when the button is pushed.
        if (!(pins & 0x01)) { 
            // Button is Pushed: Turn on GP7 (bit 7 = 1000 0000 = 0x80)
            setPin(MCP_ADDR, OLAT, 0x80);
        } else {
            // Button is Released: Turn off GP7
            setPin(MCP_ADDR, OLAT, 0x00);
        }
        
        // Short delay
        sleep_us(100);
    }
}

void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    // Send register address + data byte
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

unsigned char readPin(unsigned char address, unsigned char reg) {
    uint8_t value;
    // Write register address, then read the response
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true); // true = repeated start
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return (unsigned char)value;
}