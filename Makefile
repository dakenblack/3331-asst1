CC=gcc
CFLAGS=-g 
OFLAGS=-lpthread
DEPS=

all: client server

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

client: client.o
	$(CC) $(OFLAGS) -o $@ $< 

server: server.o
	$(CC) $(OFLAGS) -o $@ $< 

temp : temp.o
	$(CC) $(OFLAGS) -o $@ $< 

clean: 
	rm *.o temp client server
