/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (C) 2008-2009 Semihalf, Michal Hajduk and Bartlomiej Sieka
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <errno.h>
#include <sysexits.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <dev/iicbus/iic.h>

#include "i2c_rdwr.h"

#define	I2C_DEV			"/dev/iic0"
#define	I2C_MODE_NOTSET		0
#define	I2C_MODE_NONE		1
#define	I2C_MODE_STOP_START	2
#define	I2C_MODE_REPEATED_START	3
#define	I2C_MODE_TRANSFER	4

struct options {
	unsigned int	width;
	unsigned int	count;
	int	verbose;
	int	addr_set;
	int	binary;
	int	scan;
	int	skip;
	int	reset;
	int	mode;
	char	dir;
	uint32_t	addr;
	uint32_t	off;
};

static int
i2c_rdwr_transfer(char *dev, struct options i2c_opt, unsigned char *i2c_buf);

static void
print_i2c_rdwr(int error, struct options i2c_opt, unsigned char *i2c_buf);

static struct options default_opts(void);

// address - e.g. 0x77
// read: read vs write - true for read, false for write
// offset: register - e.g. 0xF4 
// count: number of bytes  - e.g. 2
// buf: buffer of size `count` - caller creates

int i2c_rdwr(
	unsigned int addr, unsigned int read, unsigned int offset,
	unsigned int count, unsigned char *buf)
{

	struct options i2c_opt = default_opts();
	unsigned char *i2c_buf = buf;
	char *dev = I2C_DEV;

	errno = 0;

	i2c_opt.addr = addr << 1;
	i2c_opt.addr_set = 1;

	i2c_opt.verbose = 0;

	i2c_opt.dir = read ? 'r' : 'w';
	i2c_opt.off = offset,
	i2c_opt.width = 8;
	i2c_opt.count = count;
	i2c_opt.mode = I2C_MODE_TRANSFER;

	if (i2c_opt.verbose)
		fprintf(stderr, "dev: %s, addr: 0x%x, r/w: %c, "
		    "offset: 0x%02x, width: %u, count: %u\n", dev,
		    i2c_opt.addr >> 1, i2c_opt.dir, i2c_opt.off,
		    i2c_opt.width, i2c_opt.count);

	int error = i2c_rdwr_transfer(dev, i2c_opt, i2c_buf);

	if (i2c_opt.verbose) {
		print_i2c_rdwr(error, i2c_opt, i2c_buf);
	}

	return error;

}

/* Print the output */
void print_i2c_rdwr(int error, struct options i2c_opt, unsigned char *i2c_buf) {

	if (error != 0) {
		fprintf(stderr, "Error?????\n");
		fprintf(stderr, "the stuff below may be garbage, idk\n");
	}

	/* Line-break the output every chunk_size bytes */
	int chunk_size = 16;

	int i = 0;
	int j = 0;
	while (i < i2c_opt.count) {
		if (i2c_opt.verbose || (i2c_opt.dir == 'r' &&
		    !i2c_opt.binary))
			fprintf (stderr, "%02hhx ", i2c_buf[i++]);

		if (i2c_opt.dir == 'r' && i2c_opt.binary) {
			fprintf(stdout, "%c", i2c_buf[j++]);
			if(!i2c_opt.verbose)
				i++;
		}
		if (!i2c_opt.verbose && (i2c_opt.dir == 'w'))
			break;
		if ((i % chunk_size) == 0)
			fprintf(stderr, "\n");
	}
	if ((i % chunk_size) != 0)
		fprintf(stderr, "\n");

}


/*
 * i2c_rdwr_transfer() - use I2CRDWR to conduct a complete i2c transfer.
 *
 * Some i2c hardware is unable to provide direct control over START, REPEAT-
 * START, and STOP operations.  Such hardware can only perform a complete
 * START-<data>-STOP or START-<data>-REPEAT-START-<data>-STOP sequence as a
 * single operation.  The driver framework refers to this sequence as a
 * "transfer" so we call it "transfer mode".  We assemble either one or two
 * iic_msg structures to describe the IO operations, and hand them off to the
 * driver to be handled as a single transfer.
 */
static int
i2c_rdwr_transfer(char *dev, struct options i2c_opt, unsigned char *i2c_buf)
{
	struct iic_msg msgs[2];
	struct iic_rdwr_data xfer;
	int fd, i;
	union {
		uint8_t  buf[2];
		uint8_t  off8;
		uint16_t off16;
	} off;

	i = 0;
	if (i2c_opt.width > 0) {
		msgs[i].flags = IIC_M_WR | IIC_M_NOSTOP;
		msgs[i].slave = i2c_opt.addr;
		msgs[i].buf   = off.buf;
		if (i2c_opt.width == 8) {
			off.off8 = (uint8_t)i2c_opt.off;
			msgs[i].len = 1;
		} else {
			off.off16 = (uint16_t)i2c_opt.off;
			msgs[i].len = 2;
		}
		++i;
	}

	/*
	 * If the transfer direction is write and we did a write of the offset
	 * above, then we need to elide the start; this transfer is just more
	 * writing that follows the one started above.  For a read, we always do
	 * a start; if we did an offset write above it'll be a repeat-start
	 * because of the NOSTOP flag used above.
	 */
	if (i2c_opt.dir == 'w')
		msgs[i].flags = IIC_M_WR | ( (i > 0) ? IIC_M_NOSTART : 0 );
	else
		msgs[i].flags = IIC_M_RD;
	msgs[i].slave = i2c_opt.addr;
	msgs[i].len   = i2c_opt.count;
	msgs[i].buf   = i2c_buf;
	++i;

	xfer.msgs = msgs;
	xfer.nmsgs = i;

	if ((fd = open(dev, O_RDWR)) == -1)
		err(1, "open(%s) failed", dev);
	if (ioctl(fd, I2CRDWR, &xfer) == -1)
		err(1, "ioctl(I2CRDWR) failed");
	close(fd);

	return (0);
}

static struct options default_opts(void)
{
	struct options i2c_opt;

	/* Default values */
	i2c_opt.addr_set = 0;
	i2c_opt.off = 0;
	i2c_opt.verbose = 1;
	i2c_opt.dir = 'r';	/* direction = read */
	i2c_opt.width = 8;
	i2c_opt.count = 1;
	i2c_opt.binary = 0;	/* ASCII text output */
	i2c_opt.scan = 0;	/* no bus scan */
	i2c_opt.skip = 0;	/* scan all addresses */
	i2c_opt.reset = 0;	/* no bus reset */
	i2c_opt.mode = I2C_MODE_NOTSET;

	return i2c_opt;
}
