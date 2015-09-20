TARGET=smarthome
OBJECTS=smarthome.o

CC=g++
CFLAGS=-Wall -Werror
LDFLAGS=

${TARGET}: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o $@

all: ${TARGET}
clean:
	rm -f ${OBJECTS} ${TARGET} core
