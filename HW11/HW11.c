#include <stdio.h>
#include "pico/stdlib.h"

int main()
{
    stdio_init_all();

    while (true) {
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            putchar(ch); 
            fflush(stdout);
        }
    }
}