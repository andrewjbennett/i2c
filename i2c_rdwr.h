// Andrew Bennett, 2019-09-22

typedef struct i2c_info *I2C;

// I2CRDWR ioctl

int i2c_rdwr(
	unsigned int addr, unsigned int read, unsigned int offset,
	unsigned int count, unsigned char *buf
);


// I2CRDWR ioctl, shorthand versions
int i2c_read(
	unsigned int addr, unsigned int offset,
	unsigned int count, unsigned char *buf);

int i2c_write(
	unsigned int addr, unsigned int offset,
	unsigned int count, unsigned char *buf);

int i2c_put(unsigned int addr, unsigned int offset, unsigned char cmd);

// These functions moved to i2c_rdwr_direct.[ch]
// // Open a given address on a given bus; returns a pointer to some info
// I2C i2c_open(unsigned int bus, unsigned int addr);
//
// // Reads directly from the file descriptor
// int i2c_read_raw(I2C i2c, unsigned int count, unsigned char *buf);
//
// // Writes directly to the file descriptor
// int i2c_write_raw(I2C i2c, unsigned int count, unsigned char *buf);
//
// // Writes command + argument, then reads `count` bytes into `buf`.
// int i2c_get_direct(I2C i2c, unsigned int command, unsigned int argument,
//     unsigned int count, unsigned char *buf);
//
// // Writes command + argument, and reads nothing.
// int i2c_put_direct(I2C i2c, unsigned int command, unsigned int argument);
