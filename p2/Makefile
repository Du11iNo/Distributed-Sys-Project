# EPI (Gijon)
# Degree in Computer Engineering
# Distributed Systems (3rd Year)
# Distributed Applications in Linux/Unix with Sun RPC


#****************************************************
# Variables
#****************************************************

CC = gcc
RPCGEN = rpcgen
CFLAGS = -I. -Wall -ggdb -I/usr/include/tirpc
LDFLAGS = -ltirpc -lpthread
CLIENT = client
SERVER = server

all : $(CLIENT) $(SERVER)

#**************************************************
# Client and server generation
#**************************************************

$(CLIENT) : client.o srvwww_clnt.o srvwww_xdr.o util.o
    $(CC) $(CFLAGS) -o $(CLIENT) client.o srvwww_clnt.o srvwww_xdr.o util.o $(LDFLAGS)

$(SERVER) : srvwww_svc.o srvwww_xdr.o util.o services.o
    $(CC) $(CFLAGS) -o $(SERVER) srvwww_svc.o srvwww_xdr.o util.o services.o $(LDFLAGS)

srvwww_svc.o : srvwww_svc.c srvwww.h
    $(CC) $(CFLAGS) -c srvwww_svc.c

services.o : services.c srvwww.h util.h
    $(CC) $(CFLAGS) -c services.c

srvwww_clnt.o : srvwww_clnt.c srvwww.h
    $(CC) $(CFLAGS) -c srvwww_clnt.c

srvwww_xdr.o : srvwww_xdr.c srvwww.h
    $(CC) $(CFLAGS) -c srvwww_xdr.c

client.o : client.c srvwww.h util.h
    $(CC) $(CFLAGS) -c client.c

util.o : util.c util.h srvwww.h
    $(CC) $(CFLAGS) -c util.c

srvwww.h srvwww_svc.c : srvwww.x
    $(RPCGEN) $<
    python3 apply_srvwww_svc_patches.py

#**************************************************
# Utilities
#**************************************************

clean :
    rm -f *.o

cleanall : clean
    rm -f srvwww_*
    rm -f srvwww.h
    rm -f client server
