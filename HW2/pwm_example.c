#include <stdio.h>
#include "pico/stdlib.h"

// to get the pwm functions
// add hardware_pwm to target_link_libraries in CMakeLists.txt
#include "hardware/pwm.h" 

#define LEDPin 25 // the built in LED on the Pico

int main()
{
    stdio_init_all();

    gpio_set_function(LEDPin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    // the clock frequency is 150MHz divided by a float from 1 to 255
    float div = 3; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // sets the clock speed
    uint16_t wrap = 50000; // when to rollover, must be less than 65535
    
    // set the PWM frequency and resolution
    // this sets the PWM frequency to 150MHz / div / wrap
    pwm_set_wrap(slice_num, wrap); 
    pwm_set_enabled(slice_num, true); // turn on the PWM

    pwm_set_gpio_level(LEDPin, wrap / 2); // set the duty cycle to 50%

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
