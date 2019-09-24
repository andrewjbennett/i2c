// Andrew Bennett, 2019-09-22
//
// Code inspired by SparkFun's SCD30 Arduino library:
// https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "scd30.h"

#include "i2c.h"

static int data_available(I2C i2c);

#define SCD30_ADDRESS 0x61

#define CMD_START_CONT_MEASUREMENT   0x0010
#define CMD_STOP_CONT_MEASUREMENT    0x0104
#define CMD_SET_MEASUREMENT_INTERVAL 0x4600
#define CMD_GET_DATA_READY_STATUS    0x0202
#define CMD_READ_MEASUREMENT         0x0300
#define CMD_AUTOMATIC_SELF_CALIB     0x5306
#define CMD_SET_FORCED_RECALIB       0x5204
#define CMD_SET_TEMP_OFFSET          0x5403
#define CMD_SET_ALTITUDE_COMP        0x5102
#define CMD_READ_FIRMWARE_VER        0xD100
#define CMD_SOFT_RESET               0xD304

struct SCD30 {
	I2C i2c_info;

	float co2;
	float humidity;
	float temperature;

	int read_co2;
	int read_humidity;
	int read_temperature;

};

SCD30 scd30_init(int bus, unsigned int interval, unsigned int pressure_offset) {

	I2C i2c = i2c_open(bus, SCD30_ADDRESS);

	// start continous measurement
	// TODO: verify that the pressure offset is sane?
	i2c_put_direct16(i2c, CMD_START_CONT_MEASUREMENT, pressure_offset, 1);

	// set measurement interval
	i2c_put_direct16(i2c, CMD_SET_MEASUREMENT_INTERVAL, interval, 1);

	// enable self-calib
	i2c_put_direct16(i2c, CMD_AUTOMATIC_SELF_CALIB, 1, 1);

	struct SCD30 *s = calloc(1, sizeof(struct SCD30));
	s->i2c_info = i2c;
	s->read_co2 = 1;
	s->read_humidity = 1;
	s->read_temperature = 1;

	return s;
}

static int data_available(I2C i2c) {

	unsigned char buf[10] = {0};

	i2c_get_direct16(i2c, CMD_GET_DATA_READY_STATUS, 0, 0, 3, buf);

	return (buf[0] == 0x00 && buf[1] == 0x01);
}

static void read_measurement(SCD30 s) {
	usleep(5000);

	if (!data_available(s->i2c_info)) {
		sleep(1);
		if (!data_available(s->i2c_info)) {
			return;
		}
	}

	// The data is provided as the direct bytes of a float, so we need
	// to temporarily save them in uint32s to put the bytes in, then
	// say later that it's actually a float.
	uint32_t co2 = 0;
	uint32_t temperature = 0;
	uint32_t humidity = 0;

	unsigned char buf[20] = {0}; // heaps big
	unsigned char *p = buf;

	i2c_get_direct16(s->i2c_info, CMD_READ_MEASUREMENT, 0, 0, 18, buf);

	// The data stream looks like this:
	//
	// [0]        [1]        [2]   [3]        [4]        [5]
	// [CO2 MMSB] [CO2 MLSB] [CRC] [CO2 LMSB] [CO2 LLSB] [CRC]
	//
	// [6]        [7]        [8]   [9]        [10]       [11]
	// [TMP MMSB] [TMP MLSB] [CRC] [TMP LMSB] [TMP LLSB] [CRC]
	//
	// [12]       [13]       [14]  [15]       [16]       [17]
	// [HMD MMSB] [HMD MLSB] [CRC] [HMD LMSB] [HMD LLSB] [CRC]
	//
	// So, bytes 0, 1, 3, 4 make up the co2 reading, bytes 6, 7, 9, 10
	// make up the temperature reading, bytes 12, 13, 15, 16 make up the
	// humiddity reading.
	// We just ignore the CRC bits (2, 5, 8, 11, 14, 17).

	co2 =         (p[0]  << 24) + (p[1]  << 16) + (p[3]  << 8) + (p[4]);
	temperature = (p[6]  << 24) + (p[7]  << 16) + (p[9]  << 8) + (p[10]);
	humidity =    (p[12] << 24) + (p[13] << 16) + (p[15] << 8) + (p[16]);

	s->co2 =         * (float *) &co2;
	s->temperature = * (float *) &temperature;
	s->humidity =    * (float *) &humidity;

	//printf("CO2: %.0f, temp: %.1f, humid: %.1f\n", s->co2, s->temperature, s->humidity);

	s->read_co2 = 0;
	s->read_temperature = 0;
	s->read_humidity = 0;
}

// Temperature in degrees Celsius.
double scd30_get_temperature(SCD30 s) {

	if (s->read_temperature == 1) {
		read_measurement(s);
	}

	s->read_temperature = 1;

	return s->temperature;
}

// Humidity in %RH.
double scd30_get_humidity(SCD30 s) {

	if (s->read_humidity == 1) {
		read_measurement(s);
	}

	s->read_humidity = 1;

	return s->humidity;
}

// CO2 in ppm.
double scd30_get_co2(SCD30 s) {

	if (s->read_co2 == 1) {
		read_measurement(s);
	}

	s->read_co2 = 1;

	return s->co2;
}
