// Andrew Bennett, 2019-09-24

#include <stdio.h>
#include <time.h>

#include "scd30.h"
#include "bmp180.h"

// using i2c-gpio
#define BUS_NUMBER 1

#ifdef DEBUG_ON
#define DEBUG 1
#else
#define DEBUG 0
#endif


static void get_data(BMP180 bmp180, SCD30 scd30);


int main(void) {

	BMP180 bmp180 = bmp180_init(BUS_NUMBER);

	long bmp180_pressure = bmp180_get_pressure(bmp180);

	if (DEBUG) printf("about to init scd30\n");
	SCD30 scd30 = scd30_init(BUS_NUMBER, 2, bmp180_pressure);

	get_data(bmp180, scd30);

	return 0;
}

static void get_data(BMP180 bmp180, SCD30 scd30) {

	if (DEBUG) printf("about to get scd30 co2\n");
	float scd30_co2 = scd30_get_co2(scd30);
	if (DEBUG) printf("about to get scd30 temp\n");
	float scd30_temperature = scd30_get_temperature(scd30);
	if (DEBUG) printf("about to get scd30 humid\n");
	float scd30_humidity = scd30_get_humidity(scd30);

	if (scd30_co2 == 0) {
		if (DEBUG) printf("co2 was zero?\n");

		scd30_co2 = scd30_get_co2(scd30);
	}

	double bmp180_temperature = bmp180_get_temperature(bmp180);
	long bmp180_pressure = bmp180_get_pressure(bmp180);

	printf("%lu, ", time(NULL));

	printf("scd30_CO2: %.0f, scd30_temp: %.1f, scd30_humid: %.1f, ",
			scd30_co2, scd30_temperature, scd30_humidity);

	printf("bmp180_temp: %.1f, bmp180_pres: %ld\n",
			bmp180_temperature, bmp180_pressure);

}
