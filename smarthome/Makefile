TARGET=smarthome
OBJECTS=smarthome.o gpio.o ../sysfs_gpio/GPIOClass.o log/log.o ../ini_parser/ini.o ../ini_parser/INIReader.o

CXX=$(CROSS_COMPILE)g++
LD=$(CROSS_COMPILE)ld
CXXFLAGS=-Wall #-Werror
LDFLAGS=-lpthread

${TARGET}: ${OBJECTS}
	${CXX} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
