// Andrew Bennett, 2019-09-22

int i2c_rdwr(
	unsigned int addr, uint8_t read, unsigned int offset,
	unsigned int count, unsigned char *buf
);
