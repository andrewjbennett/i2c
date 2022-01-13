// Andrew Bennett, 2019-09-22
// Based on code by Jashank Jeremy, 2019-09-20 ish
//
// Data from reference sheet:
// https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

#include <stdio.h>
#include <stdint.h>

#include <unistd.h>

#include <stdlib.h>
#include <math.h>

#include "bmp180.h"
#include "i2c.h"

// #define USE_STORED_CALIB

#define DEBUG 0
#define show(x) if (DEBUG) printf("%3s = %ld\n", #x, (x));

#define REG_MEASUREMENT_CONTROL 0xF4
#define REG_OUTPUT_MSB 0xF6

#define CMD_READ_TEMP '.'
#define CMD_READ_PRESSURE '4'

#define READ_TEMP_WAIT     4500 // 4.5ms
#define READ_PRES_STD_WAIT 7500 // 7.5ms

#define BMP180_ADDRESS 0x77

struct BMP180 {
	I2C i2c;
};

typedef struct calib {
	int16_t  ac1, ac2, ac3;
	uint16_t ac4, ac5, ac6;
	int16_t b1, b2, mb, mc, md;
} calib;

static long get_temperature_uncomp(BMP180);
static long get_pressure_uncomp(BMP180);

// TODO: make this dynamic eventually I guess?
static calib bmp180_get_calib(BMP180);
static void print_calib(calib k);

// Initialise stuff required for the bmp180 sensor.
BMP180 bmp180_init(int bus) {
	I2C i2c = i2c_open(bus, BMP180_ADDRESS);

	if (i2c == NULL) {
		printf("something went wrong I guess?\n");
		return NULL;
	}

	BMP180 bmp180 = calloc(1, sizeof(struct BMP180));
	bmp180->i2c = i2c;

	return bmp180 ;
}

// Returns temperature in degrees Celsius.
double bmp180_get_temperature(BMP180 bmp180) {

	calib k = bmp180_get_calib(bmp180);
	long ut = get_temperature_uncomp(bmp180);

	long x1, x2, b5, t;
	x1 = ((ut - k.ac6) * k.ac5) / (1<<15);	show (x1);
	x2 = (k.mc * (1<<11)) / (x1 + k.md);	show (x2);
	b5 = x1 + x2;	show (b5);
	t = (b5 + 8) / (1<<4);	show (t);

	return t / 10.f;
}

// Returns pressure in hPa.
double bmp180_get_pressure(BMP180 bmp180) {

	// TODO: maybe implement this?
	short oss = 0;

	calib k = bmp180_get_calib(bmp180);
	long ut = get_temperature_uncomp(bmp180);
	long up = get_pressure_uncomp(bmp180) >> (8 - oss);

	// Temperature
	long x1, x2, b5, t;
	x1 = ((ut - k.ac6) * k.ac5) / (1<<15);	show (x1);
	x2 = (k.mc * (1<<11)) / (x1 + k.md);	show (x2);
	b5 = x1 + x2;	show (b5);
	t = (b5 + 8) / (1<<4);	show (t);

	// Pressure
	long  b6, x3, b3, p;
	unsigned long b4, b7;
	b6 = b5 - 4000;	show (b6);
	x1 = (k.b2 * (b6 * b6 / (1<<12))) / (1<<11); show (x1);
	x2 = k.ac2 * b6 / (1<<11); show (x2);
	x3 = x1 + x2; show (x3);
	b3 = (((k.ac1 * 4 + x3) << oss) + 2) / 4; show (b3);
	x1 = k.ac3 * b6 / (1<<13); show (x1);
	x2 = (k.b1 * (b6 * b6 / (1<<12))) / (1<<16); show (x2);
	x3 = ((x1 + x2) + 2) / (1<<2); show (x3);
	b4 = (k.ac4 * (unsigned long)(x3 + 32768)) / (1<<15); show (b4);
	b7 = (((unsigned long)up - b3) * (50000 >> oss)); show (b7);
	p = (b7 < 0x80000000ULL)
		? ((b7 * 2) / b4)
		: ((b7 / b4) * 2)
		; show (p);
	show ((b7 * 2) / b4);
	show ((b7 / b4) * 2);

	x1 = (p / (1<<8)) * (p / (1<<8)); show (x1);
	x1 = (x1 * 3038) / (1<<16); show (x1);
	x2 = (-7357 * p) / (1<<16); show (x2);
	p += (x1 + x2 + 3791) / (1<<4); show (p);

	return p / 100.f;
}

static long get_temperature_uncomp(BMP180 bmp180) {

	i2c_put_direct8(bmp180->i2c, REG_MEASUREMENT_CONTROL, CMD_READ_TEMP, 1);

	usleep(READ_TEMP_WAIT);

	unsigned char buf[10]; // heaps big
	i2c_get_direct8(bmp180->i2c, REG_OUTPUT_MSB, 0, 0, 2, buf);

	long ut = (buf[0] << 8) + buf[1];
	return ut;
}

static long get_pressure_uncomp(BMP180 bmp180) {


	i2c_put_direct8(bmp180->i2c, REG_MEASUREMENT_CONTROL, CMD_READ_PRESSURE, 1);

	usleep(READ_PRES_STD_WAIT);

	unsigned char buf[10]; // heaps big
	i2c_get_direct8(bmp180->i2c, REG_OUTPUT_MSB, 0, 0, 2, buf);

	long up = (buf[0] << 16) + (buf[1] << 8) + 0;
	return up;
}

static calib bmp180_get_calib(BMP180 bmp180) {

	uint16_t calib_data[12] = {0};

	uint8_t start = 0xaa;
	uint8_t end   = 0xbf;

	unsigned char buf[10]; // heaps big

	uint8_t curr = start;
	int index = 0;
	while (curr < end) {
		i2c_get_direct8(bmp180->i2c, curr, 0, 0, 1, buf);
		calib_data[index] = buf[0] << 8;
		curr++;

		i2c_get_direct8(bmp180->i2c, curr, 0, 0, 1, buf);
		calib_data[index] += buf[0];
		curr++;
		index++;
	}


	calib k = {
#ifdef USE_STORED_CALIB
		.ac1 = 0x2050,
		.ac2 = 0xfb83,
		.ac3 = 0xc740,
		.ac4 = 0x8299,
		.ac5 = 0x615d,
		.ac6 = 0x4973,
		.b1  = 0x1973,
		.b2  = 0x002c,
		.mb  = 0x8000,
		.mc  = 0xd1f6,
		.md  = 0x0b32,
#else
		.ac1 = calib_data[0],
		.ac2 = calib_data[1],
		.ac3 = calib_data[2],
		.ac4 = calib_data[3],
		.ac5 = calib_data[4],
		.ac6 = calib_data[5],
		.b1  = calib_data[6],
		.b2  = calib_data[7],
		.mb  = calib_data[8],
		.mc  = calib_data[9],
		.md  = calib_data[10],
#endif
	};

	return k;
}

static void print_calib(calib k) {
	printf("ac1: %02x, ", k.ac1);
	printf("ac2: %02x, ", k.ac2);
	printf("ac3: %02x, ", k.ac3);
	printf("ac4: %02x, ", k.ac4);
	printf("ac5: %02x, ", k.ac5);
	printf("ac6: %02x, ", k.ac6);
	printf("b1 : %02x, ", k.b1 );
	printf("b2 : %02x, ", k.b2 );
	printf("mb : %02x, ", k.mb );
	printf("mc : %02x, ", k.mc );
	printf("md : %02x, ", k.md );
	printf("\n");
}
