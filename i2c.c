// Andrew Bennett, 2019-06-23
//
// Code inspired by joan2937's pigpio library:
// https://github.com/joan2937/pigpio/blob/master/pigpio.c

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "i2c.h"

#define I2C_SLAVE   0x0703
#define I2C_FUNCS   0x0705

struct i2c_info {
	int16_t fd;
	uint32_t addr;
};

static void dump_buf(unsigned int count, unsigned char *buf);

// 2-byte versions
// ---------------------------------------------------------------------

// Writes command + argument, then reads `count` bytes into `buf`.
// Assumes 2 byte command + optional 2 byte argument.
int i2c_get_direct16_orig(I2C i2c, uint16_t command, uint16_t argument,
	int has_argument, unsigned int count, unsigned char *buf) {

	unsigned char input_buf[10] = {0};
	int input_count = 2;
	input_buf[0] = (command >> 8) & 0xff;
	input_buf[1] = (command) & 0xff;

	if (has_argument) {
		input_count = 4;
		input_buf[2] = (argument >> 8) & 0xff;
		input_buf[3] = (argument) & 0xff;
	}

	i2c_write_raw(i2c, input_count, input_buf);
	int bytes = i2c_read_raw(i2c, count, buf);

	return bytes;
}

int i2c_get_direct16(I2C i2c, uint16_t command, uint16_t argument,
	int has_argument, unsigned int count, unsigned char *buf) {

	unsigned char input_buf[10] = {0};

	input_buf[0] = (command >> 8) & 0xff;
	i2c_write_raw(i2c, 1, input_buf);

	input_buf[0] = (command) & 0xff;
	i2c_write_raw(i2c, 1, input_buf);

	if (has_argument) {
		input_buf[0] = (argument >> 8) & 0xff;
		i2c_write_raw(i2c, 1, input_buf);

		input_buf[0] = (argument) & 0xff;
		i2c_write_raw(i2c, 1, input_buf);
	}

	int bytes = i2c_read_raw(i2c, count, buf);
	dump_buf(count, buf);

	return bytes;
}

// Writes command + argument, and reads nothing.
// Assumes 2 byte command + optional 2 byte argument.
int i2c_put_direct16(I2C i2c, unsigned int command, unsigned int argument, int has_argument) {
	return i2c_get_direct16(i2c, command, argument, has_argument, 0, NULL);
}

// 1-byte versions
// ---------------------------------------------------------------------

// Writes command + argument, then reads `count` bytes into `buf`.
// Assumes 1 byte command + optional 1 byte argument.
int i2c_get_direct8(I2C i2c, uint8_t command, uint8_t argument,
	int has_argument, unsigned int count, unsigned char *buf) {

	unsigned char input_buf[10] = {0};
	int input_count = 1;
	input_buf[0] = (command) & 0xff;

	if (has_argument) {
		input_count = 2;
		input_buf[1] = (argument) & 0xff;
	}

	i2c_write_raw(i2c, input_count, input_buf);
	int bytes = i2c_read_raw(i2c, count, buf);

	return bytes;
}

// Writes command + argument, and reads nothing.
// Assumes 1 byte command + optional 1 byte argument.
int i2c_put_direct8(I2C i2c, uint8_t command, uint8_t argument, int has_argument) {
	return i2c_get_direct8(i2c, command, argument, has_argument, 0, NULL);
}

// Underlying functions to directly open / read from / write to the device
// ---------------------------------------------------------------------

// Open the i2c device `bus`, for the device address `addr`.
I2C i2c_open(unsigned int bus, unsigned int addr) {
	char dev[32] = {0};
	sprintf(dev, "/dev/i2c-%u", bus);

	int fd;
	if ((fd = open(dev, O_RDWR)) < 0) {
		fprintf(stderr, "Error? Unable to open dev %s\n", dev);
		return NULL;
	}

	// set the address
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		fprintf(stderr, "Error? Unable to set address %x\n", addr);
		close(fd);
		return NULL;
	}

	I2C i2c = calloc(1, sizeof(struct i2c_info));

	i2c->fd = fd;
	i2c->addr = addr;

	return i2c;
}

// Reads directly from the file descriptor. Returns # bytes read.
int i2c_read_raw(I2C i2c, unsigned int count, unsigned char *buf) {

	if (count == 0) return 0;
	if (buf == NULL) return 0;

	int bytes_read = read(i2c->fd, buf, count);

	if (bytes_read != count) {
		fprintf(stderr, "huh? didn't read as many as we wanted? only read %d bytes (wanted %d)\n", bytes_read, count);
	}

	//printf("read: "); dump_buf(count, buf);
	return bytes_read;
}

// Writes directly to the file descriptor
int i2c_write_raw(I2C i2c, unsigned int count, unsigned char *buf) {
	if (count == 0) return 0;
	if (buf == NULL) return 0;

	int bytes_written = write(i2c->fd, buf, count);

	if (bytes_written != count) {
		fprintf(stderr, "huh? didn't write as many as we wanted? only wrote %d bytes (wanted %d)\n", bytes_written, count);
	}

	//printf("write: "); dump_buf(count, buf);
	return bytes_written;
}

// Dumps the buffer that was read to/from.
static void dump_buf(unsigned int count, unsigned char *buf) {
	for (int i = 0; i < count; i++) {
		printf("'%x' ", buf[i]);
	}
	printf("\n");
}
