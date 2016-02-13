TARGET=ini
OBJECTS=INIReader.o ini.o

CXX=$(CROSS_COMPILE)g++
LD=$(CROSS_COMPILE)ld
CXXFLAGS=-Wall -Werror
LDFLAGS=

${TARGET}: ${OBJECTS}
	${CXX} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
