#include "protocol.h"
#include <sys/socket.h>

int CreateTCPServerSocket(int port) {
    // Creates a passive socket on the given port and returns it.
    // Aborts program execution in case of error.

    int ret;
    int sock;
    struct sockaddr_in addr;

    ret = socket(PF_INET, SOCK_STREAM, 0);
    if (ret == -1) {
        perror("Error creating TCP socket");
        exit(20);
    }
    sock = ret;
    if (port != 0) {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1) {
            perror("Error assigning address to TCP socket");
            close(sock);
            exit(21);
        }
    }
    ret = listen(sock, SOMAXCONN);
    if (ret == -1) {
        perror("Error putting TCP socket in listen mode");
        close(sock);
        exit(22);
    }
    return sock;
}

int CreateUDPSocket(int port) {
    // Creates a passive socket on the given port and returns it.
    // Aborts program execution in case of error.

    int ret;
    int sock;
    struct sockaddr_in addr;

    ret = socket(PF_INET, SOCK_DGRAM, 0);
    if (ret == -1) {
        perror("Error creating UDP socket");
        exit(20);
    }
    sock = ret;
    if (port != 0) {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1) {
            perror("Error assigning address to UDP socket");
            close(sock);
            exit(21);
        }
    }
    return sock;
}

int AcceptConnection(int sock) {
    // Accepts a connection on the passive socket, or aborts execution on error.
    int ret;

    ret = accept(sock, NULL, NULL);
    if (ret == -1) {
        perror("Error accepting connection on TCP socket");
        exit(22);
    }
    return ret;
}

int CreateTCPDataSocket() {
    // Creates and returns a data socket.
    // Aborts execution in case of error.
    int sock;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error creating TCP data socket");
        exit(23);
    }
    return sock;
}

int CreateUDPDataSocket() {
    // Creates and returns a data socket.
    // Aborts execution in case of error.
    int sock;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Error creating UDP data socket");
        exit(23);
    }
    return sock;
}

int Connect(int sock, char *ip, int port) {
    // Attempts a connection to the given IP and port (IP expected as a string,
    // e.g., "127.0.0.1").
    // Aborts program execution in case of error.

    struct sockaddr_in server_addr;
    int result;

    // Initialize server_addr
    // TODO: TO BE COMPLETED BY STUDENT

    server_addr.sin_family = AF_INET;
    inet_aton(ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    result =
        connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    return result;
}

int sendStringTCP(int s, char *str, int len) {
    int sent;
    sent = send(s, str, len, 0);
    if (sent != len) {
        perror("Error sending string over TCP socket");
    }
    return sent;
}

int sendStringUDP(int s, char *str, int len, struct sockaddr_in *addr) {
    int sent;
    sent = sendto(s, str, len, 0, (struct sockaddr *)addr,
                  sizeof(struct sockaddr_in));
    if (sent != len) {
        perror("Error sending string over UDP socket");
    }
    return sent;
}

int recvStringTCP(int s, char *str, int len) {
    int received;
    received = recv(s, str, len - 1, 0);
    if (received == -1) {
        perror("Error receiving string over TCP socket");
    } else {
        // TODO: TO BE COMPLETED BY STUDENT

        if (received < len)
            str[received] = '\0';
    }
    return received;
}

int recvStringUDP(int s, char *str, int len, struct sockaddr_in *addr) {
    int received;
    int size = sizeof(struct sockaddr_in);

    received = recvfrom(s, str, len, 0, (struct sockaddr *)addr, &size);
    if (received == -1) {
        perror("Error receiving string over UDP socket");
    } else {
        // TODO: TO BE COMPLETED BY STUDENT

        if (received < len)
            str[received] = '\0';
    }
    return received;
}

void CloseSocket(int s) { close(s); }
