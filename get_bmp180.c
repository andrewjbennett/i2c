// Andrew Bennett, 2019-09-22

#include <stdio.h>
#include <time.h>

#include "bmp180.h"

int main(void) {

    BMP180 bmp180 = bmp180_init(1);

    double temperature = bmp180_get_temperature(bmp180);
    long pressure = bmp180_get_pressure(bmp180);

    printf("%lu, ", time(NULL));

    printf("bmp180_temp: %.1f, bmp180_pres: %ld\n",
            temperature, pressure);

}
