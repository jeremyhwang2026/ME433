#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Pin configuration
#define PIN_MISO   16
#define PIN_SCK    18
#define PIN_MOSI   19
#define PIN_DAC_CS 21
#define PIN_RAM_CS 20

#define SPI_PORT spi0

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void spi_ram_set_mode(uint8_t mode_byte) {
    uint8_t cmd[2] = { 0x01, mode_byte };
    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, cmd, 2);
    cs_deselect(PIN_RAM_CS);
}

void spi_ram_write_sequential(uint16_t address, uint8_t *data, size_t length) {
    uint8_t header[3] = {
        0x02,                
        (uint8_t)(address >> 8),
        (uint8_t)(address & 0xFF)
    };

    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_write_blocking(SPI_PORT, data, length);
    cs_deselect(PIN_RAM_CS);
}

void spi_ram_read_sequential(uint16_t address, uint8_t *buffer, size_t length) {
    uint8_t header[3] = {
        0x03,                 
        (uint8_t)(address >> 8), 
        (uint8_t)(address & 0xFF)
    };

    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, header, 3);
    spi_read_blocking(SPI_PORT, 0x00, buffer, length);
    cs_deselect(PIN_RAM_CS);
}

void write_to_dac_raw(uint8_t high_byte, uint8_t low_byte) {
    uint8_t buffer[2] = { high_byte, low_byte };
    cs_select(PIN_DAC_CS);
    spi_write_blocking(SPI_PORT, buffer, 2);
    cs_deselect(PIN_DAC_CS);
}

int main() {
    stdio_init_all();
    
    spi_init(SPI_PORT, 2 * 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_DAC_CS); gpio_set_dir(PIN_DAC_CS, GPIO_OUT); gpio_put(PIN_DAC_CS, 1);
    gpio_init(PIN_RAM_CS); gpio_set_dir(PIN_RAM_CS, GPIO_OUT); gpio_put(PIN_RAM_CS, 1);

    spi_ram_set_mode(0x40); 


    static uint8_t local_wave_buffer[2000];
    
    for (int i = 0; i < 1000; i++) {
        float sample_val = (float)i / 1000.0f;
        uint16_t raw_dac_value = (uint16_t)(511.5f + 511.5f * sinf(2.0f * M_PI * sample_val));
        uint16_t dac_packet = 0x3000 | (raw_dac_value << 2);
        local_wave_buffer[2 * i]     = (uint8_t)(dac_packet >> 8);
        local_wave_buffer[(2 * i) + 1] = (uint8_t)(dac_packet & 0xFF);
    }

    spi_ram_write_sequential(0x0000, local_wave_buffer, 2000);
    for(int i = 0; i < 2000; i++) { local_wave_buffer[i] = 0; }
    uint16_t point_index = 0;
    uint8_t active_dac_bytes[2];

    while (1) {
        uint16_t ram_target_address = point_index * 2;
        spi_ram_read_sequential(ram_target_address, active_dac_bytes, 2);
        write_to_dac_raw(active_dac_bytes[0], active_dac_bytes[1]);

        point_index++;
        if (point_index >= 1000) {
            point_index = 0;
        }

        sleep_ms(1);
    }

    return 0;
}