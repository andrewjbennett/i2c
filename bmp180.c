#include <stdio.h>
#include <stdint.h>

#include <unistd.h>

#include <stdlib.h>
#include <math.h>

#include "i2c_rdwr.h"

typedef struct calib {
	int16_t  ac1, ac2, ac3;
	uint16_t ac4, ac5, ac6;
	int16_t b1, b2, mb, mc, md;
} calib;

void bmp180(int UT, int UP);

//#define show(x) printf("%3s = %ld\n", #x, (x));
#define show(x) (x);

#define REG_MEASUREMENT_CONTROL 0xF4
#define REG_OUTPUT_MSB 0xF6

#define CMD_READ_TEMP '.'
#define CMD_READ_PRESSURE '4'

#define INPUT_SIZE  1
#define OUTPUT_SIZE 2

#define BMP180_ADDRESS 0x77

#define READ 1
#define WRITE 0

int main (int argc, char *argv[])
{
	int address = BMP180_ADDRESS;

	char buf[10]; // heaps big

	buf[0] = CMD_READ_TEMP;
	i2c_rdwr(address, WRITE, REG_MEASUREMENT_CONTROL, INPUT_SIZE, buf);
	sleep(1);

	i2c_rdwr(address, READ, REG_OUTPUT_MSB, OUTPUT_SIZE, buf);
	//print_buf(buf, 2);
	int UT = (buf[0] << 8) + buf[1];

	sleep(1);
	buf[0] = CMD_READ_PRESSURE;
	i2c_rdwr(address, WRITE, REG_MEASUREMENT_CONTROL, INPUT_SIZE, buf);

	sleep(1);
	i2c_rdwr(address, READ, REG_OUTPUT_MSB, OUTPUT_SIZE, buf);
	int UP = (buf[0] << 16) + (buf[1] << 8) + 0;

	bmp180(UT, UP);

	return 0;
}

void bmp180(int UT, int UP) {

	short    oss = 0;

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

	long ut = UT;
	long up = UP >> (8 - oss);

	long x1, x2, b5, t;
	x1 = ((ut - k.ac6) * k.ac5) / (1<<15);	show (x1);
	x2 = (k.mc * (1<<11)) / (x1 + k.md);	show (x2);
	b5 = x1 + x2;	show (b5);
	t = (b5 + 8) / (1<<4);	show (t);

	//printf ("temperature = %.1lf'C\n", t / 10.f);

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

	//printf ("pressure = %.0lf hPa\n", p / 100.f);

	//double p0 = 101325.f;
	double p0 = 101900.f;
	double a = 44330.f * (1.f - pow(p/p0, -5.255f));

	printf ("temperature = %.1lf'C\n", t / 10.f);
	printf ("pressure    = %.0lf hPa\n", p / 100.f);
	printf ("altitude    = %.0lf m\n", a);
}


