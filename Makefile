CC = gcc
BINARIES = PingClient

all: ${BINARIES}

PingClient: PingClient.c
	${CC} $^ -o $@

clean:
	/bin/rm -f ${BINARIES} *.o 