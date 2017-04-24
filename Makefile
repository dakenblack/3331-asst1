CC=gcc
CFLAGS=-g
DEPS=

all: client server

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

client: client.o
	$(CC) -o $@ $< 

server: server.o
	$(CC) -o $@ $< 

temp : temp.o
	$(CC) -o $@ $< 

clean: 
	rm *.o temp client server
