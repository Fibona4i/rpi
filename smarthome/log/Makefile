TARGET=log
OBJECTS=log.o

CC=g++
CFLAGS=-Wall -Werror
LDFLAGS=

${TARGET}: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
