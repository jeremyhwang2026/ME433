#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

const uint PICO_LED = 17;

// MPU6050 Registers
#define MPU_ADDR 0x68
#define ACCEL_XOUT_H 0x3B
#define PWR_MGMT_1 0x6B
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define WHO_AM_I 0x75

void mpu6050_init();
void mpu6050_read_raw(int16_t *accelX, int16_t *accelY);
void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char reg);
void drawLine(int x0, int y0, int x1, int y1, int color);

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000); // 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    ssd1306_setup();
    mpu6050_init();

    gpio_init(PICO_LED);
    gpio_set_dir(PICO_LED, GPIO_OUT);

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

        int16_t ax, ay;
        mpu6050_read_raw(&ax, &ay);

        float x_g = ax * 0.000061f;
        float y_g = ay * 0.000061f;

        ssd1306_clear();

        int end_x = 64 + (int)(x_g * -40);
        int end_y = 16 + (int)(y_g * 20);

        drawLine(64, 16, end_x, end_y, 1);

        ssd1306_update();
        sleep_ms(10); // 100Hz
    }
}

void mpu6050_init()
{
    uint8_t id = readPin(MPU_ADDR, WHO_AM_I);
    if (id != 0x68 && id != 0x98)
    {
        while (1)
        { // Error
            gpio_put(PICO_LED, 1);
            sleep_ms(100);
            gpio_put(PICO_LED, 0);
            sleep_ms(100);
        }
    }

    setPin(MPU_ADDR, PWR_MGMT_1, 0x00);   // Wake up
    setPin(MPU_ADDR, ACCEL_CONFIG, 0x00); // 2g sensitivity
    setPin(MPU_ADDR, GYRO_CONFIG, 0x18);  // 2000 dps sensitivity
}

void mpu6050_read_raw(int16_t *accelX, int16_t *accelY)
{
    uint8_t buffer[14];
    uint8_t reg = ACCEL_XOUT_H;

    i2c_write_blocking(i2c_default, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MPU_ADDR, buffer, 14, false);

    *accelX = (buffer[0] << 8) | buffer[1];
    *accelY = (buffer[2] << 8) | buffer[3];
}

void setPin(unsigned char address, unsigned char reg, unsigned char value)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

unsigned char readPin(unsigned char address, unsigned char reg)
{
    uint8_t value;
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return (unsigned char)value;
}

void drawLine(int x0, int y0, int x1, int y1, int color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        ssd1306_drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}