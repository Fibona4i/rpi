TARGET=gpio
OBJECTS=GPIOClassTest.o GPIOClass.o

CC=g++
CFLAGS=-Wall -Werror
LDFLAGS=

${TARGET}: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
