#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef PROTOCOL_H
#define PROTOCOL_H

int CreateTCPServerSocket(int port);
int CreateUDPSocket(int port);
int AcceptConnection(int sock);
int CreateTCPDataSocket();
int CreateUDPDataSocket();
int Connect(int sock, char *ip, int port);
int sendStringTCP(int s, char *str, int len);
int sendStringUDP(int s, char *str, int len, struct sockaddr_in *addr);
int recvStringTCP(int s, char *str, int len);
int recvStringUDP(int s, char *str, int len, struct sockaddr_in *addr);
void CloseSocket(int s);

#endif
