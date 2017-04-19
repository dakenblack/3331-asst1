CC=gcc
CFlAGS=-I.
DEPS=

all: client server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: client.o
	$(CC) -o $@ $< $(CFLAGS)

server: server.o
	$(CC) -o $@ $< $(CFLAGS)
