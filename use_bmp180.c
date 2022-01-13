// Andrew Bennett, 2019-09-22

#include <stdio.h>

#include "bmp180.h"

int main(void) {

    BMP180 bmp180 = bmp180_init(1);
    double temperature = bmp180_get_temperature(bmp180);
    printf ("temperature = %.1lf'C\n", temperature);

    double pressure = bmp180_get_pressure(bmp180);
    printf ("pressure    = %.0lf hPa\n", pressure);
}
