CC = arm-linux-gnueabihf-gcc
CFLAGS = -mfloat-abi=hard
LDFLAGS = -static -lssl -lcrypto
SRCS = \
	accesscontrol.c
OBJS = ${SRCS:.c=.o}

all: accesscontrol

accesscontrol: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS}

clean:
	rm -f *.o accesscontrol

test:
	qemu-arm-static ./accesscontrol
