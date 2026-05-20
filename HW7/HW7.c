#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define PIN_MISO 16 // unused here
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_CS   20

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


void write_to_dac(uint8_t channel, uint16_t value) {
    if (value > 1023) value = 1023;
    uint16_t packet = 0x3000; 

    if (channel == 1) {
        packet |= 0x8000; 
    }

    packet |= (value << 2);
    uint8_t buffer[2];
    buffer[0] = (uint8_t)(packet >> 8);
    buffer[1] = (uint8_t)(packet & 0xFF);
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, buffer, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();

    printf("Initializing SPI clock to 1MHz for MCP4912 Waveform Generator\n");

    spi_init(SPI_PORT, 1000 * 1000);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_put(PIN_CS, 1);
    gpio_set_dir(PIN_CS, GPIO_OUT);

    while (1) {
        uint64_t current_time_us = time_us_64();
        float t = (float)current_time_us / 1000000.0f; 


        uint16_t sine_val = (uint16_t)(511.5f + 511.5f * sinf(2.0f * M_PI * 2.0f * t));
        write_to_dac(0, sine_val);

        float phase = fmodf(t, 1.0f); 
        uint16_t tri_val = 0;

        if (phase < 0.5f) {
            tri_val = (uint16_t)(phase * 2.0f * 1023.0f);
        } else {
            tri_val = (uint16_t)((1.0f - phase) * 2.0f * 1023.0f);
        }
        write_to_dac(1, tri_val);

        sleep_ms(10); 
    }

    return 0;
}
