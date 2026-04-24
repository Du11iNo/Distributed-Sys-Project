all: peer-client server

peer-client: peer-client.c protocol.c protocol.h
	gcc -o peer-client peer-client.c protocol.c utils.c

server: server.c protocol.c protocol.h
	gcc -o server server.c protocol.c utils.c

clean:
	rm -f peer-client server
