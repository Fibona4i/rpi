TARGET=smarthome
OBJECTS=smarthome.o ../sysfs_gpio/GPIOClass.o log/log.o

CC=g++
CFLAGS=-Wall -Werror
LDFLAGS=-lpthread

${TARGET}: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
