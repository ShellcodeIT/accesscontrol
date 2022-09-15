CC = arm-linux-gnueabihf-gcc
CFLAGS = -mfloat-abi=hard -I./include/
LDFLAGS = -lssl -lcrypto
SRCS = \
	accesscontrol.c \
	ssl.c

OBJS = ${SRCS:.c=.o}

all: accesscontrol

accesscontrol: ${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}

clean:
	rm -f *.o accesscontrol

test:
	qemu-arm-static ./accesscontrol
