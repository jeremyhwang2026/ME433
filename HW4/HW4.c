#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

const uint PICO_LED = 17;

void drawChar(unsigned char x, unsigned char y, char c);
void drawString(unsigned char x, unsigned char y, char *msg);

int main()
{
    sleep_ms(250);

    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);      // 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    ssd1306_setup();

    gpio_init(PICO_LED);
    gpio_set_dir(PICO_LED, GPIO_OUT);

    char volt_buffer[50];
    char fps_buffer[50];

    absolute_time_t next_heartbeat = get_absolute_time();
    bool heart_state = false;

    while (true)
    {
        uint32_t t_start = to_us_since_boot(get_absolute_time());

        if (absolute_time_diff_us(get_absolute_time(), next_heartbeat) < 0)
        {
            heart_state = !heart_state;
            gpio_put(PICO_LED, heart_state);
            next_heartbeat = make_timeout_time_ms(500);
        }

        ssd1306_clear();

        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / (1 << 12); // Convert 12-bit to 3.3V scale

        sprintf(volt_buffer, "ADC0: %.3f V", voltage);
        drawString(10, 5, volt_buffer);

        uint32_t t_end = to_us_since_boot(get_absolute_time());
        float fps = 1000000.0f / (t_end - t_start);
        sprintf(fps_buffer, "FPS: %.1f", fps);
        drawString(10, 20, fps_buffer);

        ssd1306_update();
    }
}

// Draw a single 5x8 character at (x,y)
void drawChar(unsigned char x, unsigned char y, char c)
{
    // Find character in the font table (starts at 0x20 / space)
    int char_idx = c - 0x20;

    // Loop through the 5 columns of the character
    for (int col = 0; col < 5; col++)
    {
        unsigned char col_data = ASCII[char_idx][col];
        // Loop through the 8 bits (rows) of each column
        for (int row = 0; row < 8; row++)
        {
            // Check if the bit at this row is set
            if ((col_data >> row) & 1)
            {
                ssd1306_drawPixel(x + col, y + row, 1);
            }
            else
            {
                ssd1306_drawPixel(x + col, y + row, 0);
            }
        }
    }
}

// Draw a string by iterating through characters
void drawString(unsigned char x, unsigned char y, char *msg)
{
    int i = 0;
    while (msg[i] != '\0')
    {
        // Move 6 pixels horizontally for each char (5 for font + 1 for space)
        drawChar(x + (i * 6), y, msg[i]);
        i++;
    }
}