CC=gcc
CFLAGS=-I. -pthread -g
DEPS = queue.c util.c protocol.c
OBJ = srvwww.o queue.o client.o util.o protocol.o

all: srvwww client

srvwww: srvwww.c $(DEPS)
    $(CC) -o $@ $^ $(CFLAGS)

client: client.c util.c util.h protocol.c protocol.h
    $(CC) -o $@ $^ $(CFLAGS)

clean:
    rm *.o
