# $FreeBSD$

PROG=	bmp180
SRCS = i2c_rdwr.c i2c_rdwr.h bmp180.c bmp180.h use_bmp180.c

WARNS?=	2

LDADD += -lm

.include <bsd.prog.mk>
