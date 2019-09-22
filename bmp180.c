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
#include "i2c_rdwr.h"

typedef struct calib {
	int16_t  ac1, ac2, ac3;
	uint16_t ac4, ac5, ac6;
	int16_t b1, b2, mb, mc, md;
} calib;

void bmp180(int UT, int UP);

#define DEBUG 0

#define show(x) if (DEBUG) printf("%3s = %ld\n", #x, (x));
//#define show(x) (x);


#define REG_MEASUREMENT_CONTROL 0xF4
#define REG_OUTPUT_MSB 0xF6

#define CMD_READ_TEMP '.'
#define CMD_READ_PRESSURE '4'

#define READ_TEMP_WAIT     4500 // 4.5ms
#define READ_PRES_STD_WAIT 7500 // 7.5ms

#define INPUT_SIZE  1
#define OUTPUT_SIZE 2

#define BMP180_ADDRESS 0x77

#define READ 1
#define WRITE 0

static long get_temperature_uncomp(void);
static long get_pressure_uncomp(void);

// TODO: make this dynamic eventually I guess?
static calib get_calib(void);


// Returns temperature in degrees Celsius.
double get_temperature(void) {

	calib k = get_calib();
	long ut = get_temperature_uncomp();

	long x1, x2, b5, t;
	x1 = ((ut - k.ac6) * k.ac5) / (1<<15);	show (x1);
	x2 = (k.mc * (1<<11)) / (x1 + k.md);	show (x2);
	b5 = x1 + x2;	show (b5);
	t = (b5 + 8) / (1<<4);	show (t);

	return t / 10.f;
}

// Returns pressure in hPa.
double get_pressure(void) {

	// TODO: maybe implement this?
	short oss = 0;

	calib k = get_calib();
	long ut = get_temperature_uncomp();
	long up = get_pressure_uncomp();

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
	b7 = ((unsigned long)up - b3) * (50000 >> oss); show (b7);
	p = (b7 < 0x80000000ULL)
		? (b7 * 2) / b4
		: (b7 / b4) * 2
		; show (p);
	show ((b7 * 2) / b4);
	show ((b7 / b4) * 2);

	x1 = (p / (1<<8)) * (p / (1<<8)); show (x1);
	x1 = (x1 * 3038) / (1<<16); show (x1);
	x2 = (-7357 * p) / (1<<16); show (x2);
	p += (x1 + x2 + 3791) / (1<<4); show (p);

	return p / 100.f;
}

static long get_temperature_uncomp(void) {
	char buf[10]; // heaps big

	buf[0] = CMD_READ_TEMP;
	i2c_rdwr(BMP180_ADDRESS, WRITE, REG_MEASUREMENT_CONTROL, INPUT_SIZE, buf);
	usleep(READ_TEMP_WAIT);

	i2c_rdwr(BMP180_ADDRESS, READ, REG_OUTPUT_MSB, OUTPUT_SIZE, buf);
	//print_buf(buf, 2);
	long ut = (buf[0] << 8) + buf[1];

	return ut;
}

static long get_pressure_uncomp(void) {
	char buf[10]; // heaps big

	buf[0] = CMD_READ_PRESSURE;
	i2c_rdwr(BMP180_ADDRESS, WRITE, REG_MEASUREMENT_CONTROL, INPUT_SIZE, buf);

	usleep(READ_PRES_STD_WAIT);
	i2c_rdwr(BMP180_ADDRESS, READ, REG_OUTPUT_MSB, OUTPUT_SIZE, buf);
	int up = (buf[0] << 16) + (buf[1] << 8) + 0;

	return up;
}

static calib get_calib(void) {
	calib k =
	{
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
	};
	return k;
}
