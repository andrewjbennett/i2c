// Andrew Bennett, 2019-09-23

#include <stdint.h>

typedef struct i2c_info *I2C;

// Open a given address on a given bus; returns a pointer to some info
I2C i2c_open(unsigned int bus, unsigned int addr);

// Reads directly from the file descriptor
int i2c_read_raw(I2C i2c, unsigned int count, unsigned char *buf);

// Writes directly to the file descriptor
int i2c_write_raw(I2C i2c, unsigned int count, unsigned char *buf);

// 1-byte versions
// Writes command + argument, then reads `count` bytes into `buf`.
int i2c_get_direct8(I2C i2c, uint8_t command, uint8_t argument,
	int has_argument, unsigned int count, unsigned char *buf);

// Writes command + argument, and reads nothing.
int i2c_put_direct8(I2C i2c, uint8_t command, uint8_t argument, int has_argument);

// 2-byte versions
// Writes command + argument, then reads `count` bytes into `buf`.
int i2c_get_direct16(I2C i2c, uint16_t command, uint16_t argument,
	int has_argument, unsigned int count, unsigned char *buf);

// Writes command + argument, and reads nothing.
int i2c_put_direct16(I2C i2c, unsigned int command, unsigned int argument, int has_argument);

