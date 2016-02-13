TARGET=smarthome
OBJECTS=smarthome.o gpio.o ../sysfs_gpio/GPIOClass.o log/log.o ../ini_parser/ini.o ../ini_parser/INIReader.o

CC=g++
#CFLAGS=-Wall -Werror
LDFLAGS=-lpthread

${TARGET}: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
