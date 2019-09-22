// Andrew Bennett, 2019-09-22

#include <stdio.h>

#include "bmp180.h"

int main(void) {

    double temperature = get_temperature();
    printf ("temperature = %.1lf'C\n", temperature);

    double pressure = get_pressure();
    printf ("pressure    = %.0lf hPa\n", pressure);
}
