
// Header files for socket manipulation
#include "protocol.h"
#include "util.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//
// GLOBAL VARIABLES
//

// IP of the srvwww process
char *ip_srvwww;

// Port on which the srvwww process listens
int srvwww_port;

// Number of reader threads
int nthreads;

// name of the request source file
char *req_file;

// name of the log file
char *log_file;

// Maximum size of chunks downloaded via UDP
int chunk_max_size;

// file handlers
FILE *req_file_handle;
FILE *log_file_handle;

// mutexes for exclusive access to the request file and the log file
pthread_mutex_t mutex_req_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log_file = PTHREAD_MUTEX_INITIALIZER;

void process_arguments(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr,
                "Usage: %s ip_srvwww srvwww_port req_file nthreads log_file "
                "chunk_max_size\n",
                argv[0]);
        exit(1);
    }
    ip_srvwww = strdup(argv[1]);
    if (!validate_ip(argv[1])) {
        fprintf(stderr, "Error: The provided IP is not valid\n");
        exit(2);
    }
    if (!validate_number(argv[2])) {
        fprintf(stderr, "Error: The port value is not a number\n");
        exit(3);
    }
    srvwww_port = atoi(argv[2]);
    if ((srvwww_port < 1024) || (srvwww_port > 65535)) {
        fprintf(stderr,
                "Error: The port must be greater or equal to 1024 and less "
                "or equal to 65535\n");
        exit(4);
    }
    req_file = argv[3];
    if (!validate_number(argv[4])) {
        fprintf(stderr, "Error: The thread count parameter is not a number\n");
        exit(6);
    }
    nthreads = atoi(argv[4]);
    if (nthreads < 1) {
        fprintf(stderr, "Error: The number of client threads must be greater "
                        "or equal to 1\n");
        exit(7);
    }
    log_file = argv[5];

    if (!validate_number(argv[6])) {
        fprintf(stderr,
                "Error: The maximum chunk size parameter is not a number\n");
        exit(8);
    }
    chunk_max_size = atoi(argv[6]);
    if (chunk_max_size < 1) {
        fprintf(
            stderr,
            "Error: The maximum chunk size must be greater or equal to 1\n");
        exit(9);
    }
}

void print_log(const char *format, ...) {
    // Auxiliary function to print messages to the log file.
    //
    // It is used like printf, receiving a variable number of arguments
    // and a format string. For example:
    //
    // print_log("Error: Could not open file %s\n", filename);

    char buffer[MSG_SIZE];
    va_list args;

    // This block accesses the received format string and the list
    // of arguments that follow it, and passes everything to vsnprintf
    // so that the string is formatted properly.
    // The result (stored in buffer) is the message we want to write
    // to the log file.
    va_start(args, format);
    vsnprintf(buffer, MSG_SIZE, format, args);
    va_end(args);

    // Write the message in buffer to the log file, protecting
    // access with the appropriate mutex.
    // TODO: TO BE COMPLETED BY STUDENT

    pthread_mutex_lock(&mutex_log_file);

    fprintf(log_file_handle, "%s", buffer);

    pthread_mutex_unlock(&mutex_log_file);
}

void exit_successfully(int s) { exit(0); }

char *send_request(char *request, struct sockaddr_in *server_addr,
                   int is_stream) {
    /*
        This function sends a message (parameter 'request') to the server,
        using TCP or UDP depending on the value of 'is_stream'.
        The server IP and port are in the parameter server_addr.

        Once the request is sent, it waits to receive a response and returns
        that response in a dynamically allocated string (using strndup).

        For network communication, it uses the functions in protocol.c.

        Whoever calls this function must free the returned buffer
        once they are done using it.
    */

    int sock_dat;
    int sent, received;

    char response[MAX_RESPONSE_SIZE];
    char *s;

    bzero(response, MAX_RESPONSE_SIZE);
    if (is_stream) {
        // TODO: TO BE COMPLETED BY STUDENT

        sock_dat = CreateTCPDataSocket();
        char *ip = inet_ntoa(server_addr->sin_addr);
        short port = ntohs(server_addr->sin_port);

        Connect(sock_dat, ip, port);
        sent = sendStringTCP(sock_dat, request, strlen(request));
        if (sent < 0) {
            print_log("Error: Could not send request to server\n");
            exit(15);
        }
        received = recvStringTCP(sock_dat, response, sizeof(response));
        if (received < 0) {
            print_log("Error: Could not receive response from server\n");
            exit(15);
        }
    } else // the request is sent via UDP socket
    {
        // TODO: TO BE COMPLETED BY STUDENT

        sock_dat = CreateUDPDataSocket();

        sent = sendStringUDP(sock_dat, request, strlen(request), server_addr);
        if (sent < 0) {
            print_log("Error: Could not send request to server\n");
            exit(15);
        }
        received =
            recvStringUDP(sock_dat, response, sizeof(response), server_addr);
        if (received < 0) {
            print_log("Error: Could not receive response from server\n");
            exit(15);
        }
    }
    close(sock_dat);
    s = (char *)malloc(MAX_RESPONSE_SIZE * sizeof(char));
    if (s == NULL) {
        print_log(
            "Error sending request: Could not allocate memory to duplicate "
            "the response\n");
        exit(15);
    }
    // TODO: TO BE COMPLETED BY STUDENT

    strcpy(s, response);
    if (s == NULL) {
        print_log("Error: Could not allocate memory to response\n");
        exit(15);
    }
    return s;
}

void download_multimedia(struct sockaddr_in *p, char *filename) {
    /*
        This function downloads the images referenced inside the file
        specified in the parameter 'filename'. The images will be downloaded
        using the UDP protocol in chunks of maximum size 'chunk_max_size'.
    */

    FILE *fp_in;            // to read the input file
    int fd_out;             // to write to the output file
    char line[LINE_SIZE];   // to store each line read from the file
    char request[MSG_SIZE]; // to build the requests to send
    char *s;                // to store the server response
    long int file_size;     // to store the size of the file to download
    long int offset;        // to store the offset inside the file
    char *pos;
    char *token;

    // Open the file for reading
    if ((fp_in = fopen(filename, "r")) == NULL) {
        print_log("Error: Could not open file %s for reading\n", filename);
        exit(18);
    }

    // Read the file line by line
    while (fgets(line, LINE_SIZE, fp_in) != NULL) {
        pos = strstr(line, "<img ");
        if (pos == NULL) {
            pos = strstr(line, "<IMG ");
        }

        if (pos != NULL) // there is an image reference in the line
        {
            pos += 5; // skip "<img " or "<IMG "
            token = NULL;

            // extract the string until the end of the tag
            token = strtok_r(pos, ">\n\r ", &s);
            if (token != NULL) {
                sprintf(request, "EXISTFILE %s", token);
                s = send_request(request, p, FALSE);

                if (strcmp(s, "200") == 0) {
                    free(s);
                    sprintf(request, "GETSIZE %s", token);
                    s = send_request(request, p, FALSE);

                    if (s != NULL) {
                        sscanf(s, "%ld", &file_size);
                        free(s);

                        if (file_size >= 0) {
                            // Open local file to write the received data
                            fd_out = open(token, O_WRONLY | O_CREAT,
                                          0644); // permissions rw-r--r--
                            if (fd_out < 0) {
                                print_log("Error: Could not open file %s for "
                                          "writing\n",
                                          token);
                                exit(21);
                            }

                            // Iterate through the chunks to receive
                            offset = 0; // initial offset
                            while (offset < file_size) {
                                // Prepare request to obtain the chunk and send
                                // it
                                if ((offset + chunk_max_size) > file_size)
                                    chunk_max_size =
                                        file_size - offset; // adjust chunk size
                                                            // at end of file

                                sprintf(request, "GETCHUNK %s %ld %d", token,
                                        offset, chunk_max_size);
                                s = send_request(request, p, FALSE);

                                if (s != NULL) {
                                    // Write the received chunk to the file
                                    write(fd_out, s, chunk_max_size);
                                    free(s);
                                    offset +=
                                        chunk_max_size; // increment offset
                                } else                  // error obtaining chunk
                                {
                                    print_log("Error: Could not obtain chunk "
                                              "of file %s\n",
                                              token);
                                    close(fd_out);
                                    exit(23);
                                }
                            }
                            close(fd_out);
                        } else {
                            print_log(
                                "Error: File size for %s cannot be negative\n",
                                token);
                        }
                    } else // error obtaining file size
                    {
                        print_log("Error: Could not obtain file size for %s\n",
                                  token);
                    }
                } else {
                    free(s);
                    print_log("Error: File %s does not exist on the server\n",
                              token);
                }
            }
        }
    }
    fclose(fp_in);
}

void *download_thread(struct sockaddr_in *p) {
    int received;                   // to store the number of bytes received
    long int file_size;             // to store the size of the file to download
    char request[MSG_SIZE];         // to build the requests to send
    char buffer[MAX_RESPONSE_SIZE]; // to read a line from the request file
    char *s;                        // to store the server response
    int fd_out;                     // to write to the output file
    char *pt = NULL;                // to separate tokens from the read line
    char *loc;                      // to use strtok_r
    char *filename = NULL;          // to store the filename
    unsigned char is_stream; // to indicate whether the request uses TCP or UDP
    long int offset;         // to store the file offset

    // Lock before reading a line from the request file
    pthread_mutex_lock(&mutex_req_file);
    fprintf(stderr, "Reader thread before entering loop\n");

    while (fgets(buffer, MAX_RESPONSE_SIZE, req_file_handle) != NULL) {
        // Unlock after reading the line
        pthread_mutex_unlock(&mutex_req_file);

        // Remove newline
        // The read line is of the form "filename T\n" or "filename U\n"
        buffer[strlen(buffer) - 1] = '\0';

        // Separate into tokens, using space and newline as delimiters
        pt = strtok_r(buffer, " \n", &loc);

        // First token: filename
        if (pt == NULL) {
            print_log("Error: Could not obtain filename from request\n");
            exit(16);
        }
        filename = strdup(pt);
        if (filename == NULL) {
            print_log(
                "Error: Could not allocate memory for filename in request\n");
            exit(17);
        }

        // Second token: "T" or "U"
        pt = strtok_r(NULL, " \n", &loc);

        // Assign is_stream appropriately
        // TODO: TO BE COMPLETED BY STUDENT

        if (pt == NULL) {
            print_log("Error: Could not obtain stream type from request\n");
            exit(17);
        }
        is_stream = !strcmp(pt, "T") ? 'T' : 'U';

        fprintf(stderr, "Reader thread before sending request for file %s\n",
                filename);

        // Prepare request to check if file exists, and send it
        // TODO: TO BE COMPLETED BY STUDENT

        sprintf(request, "EXISTFILE %s", filename);
        s = send_request(request, p, !strcmp(pt, "T") ? TRUE : FALSE);

        if (strcmp(s, "200") == 0) {
            free(s);

            // Depending on whether the download is via TCP or UDP
            // TODO: TO BE COMPLETED BY STUDENT

            if (is_stream == 'T') {

                sprintf(request, "GETFILE %s", filename);
                int sock_dat = CreateTCPDataSocket();

                if (!Connect(sock_dat, inet_ntoa(p->sin_addr),
                             ntohs(p->sin_port))) {
                    print_log("Error: Could not connect to server\n");
                    exit(16);
                }

                if (sendStringTCP(sock_dat, request, strlen(request)) > 0) {
                    if ((fd_out = open(filename, O_WRONLY | O_CREAT, 0644)) <
                        0) {
                        print_log("Error: Could not create file %s\n",
                                  filename);
                        exit(15);
                    }
                    while ((received = recvStringTCP(sock_dat, buffer,
                                                     sizeof(buffer))) > 0) {
                        write(fd_out, buffer, received);
                    }
                    close(fd_out);
                }
                CloseSocket(sock_dat);
            } else // download is via UDP
            {
                // Prepare request to obtain file size, and send it
                // TODO: TO BE COMPLETED BY STUDENT

                sprintf(request, "GETSIZE %s", filename);
                s = send_request(request, p, FALSE);

                if (s != NULL) {
                    // Extract file size from response
                    sscanf(s, "%ld", &file_size);
                    free(s);

                    if (file_size >= 0) {
                        // Open local file to write received data
                        fd_out = open(filename, O_WRONLY | O_CREAT,
                                      0644); // permissions rw-r--r--
                        if (fd_out < 0) {
                            print_log(
                                "Error: Could not open file %s for writing\n",
                                filename);
                            exit(20);
                        }

                        // Iterate through chunks to receive
                        offset = 0; // initial offset
                        while (offset < file_size) {
                            // Prepare request to obtain chunk, and send it
                            // TODO: TO BE COMPLETED BY STUDENT

                            sprintf(request, "GETCHUNK %s %ld %d", filename,
                                    offset, chunk_max_size);
                            s = send_request(request, p, FALSE);

                            if (s != NULL) {
                                // Write received chunk to file
                                write(fd_out, s, chunk_max_size);
                                free(s);
                                offset += chunk_max_size; // increment offset
                            } else // error obtaining chunk
                            {
                                print_log("Error: Could not obtain chunk of "
                                          "file %s\n",
                                          filename);
                                close(fd_out);
                                exit(22);
                            }
                        }
                        close(fd_out);

                        if (file_size > 0) {
                            // Call function to download images referenced
                            // inside the file
                            download_multimedia(p, filename);
                        }
                    } else // file_size is negative
                    {
                        print_log(
                            "Error: File size for %s cannot be negative\n",
                            filename);
                    }
                } else // error obtaining file size
                {
                    print_log("Error: Could not obtain file size for %s\n",
                              filename);
                }
            }
        } else {
            free(s);
            print_log("Error: File %s does not exist on the server\n",
                      filename);
        }

        fprintf(stderr, "Reader thread before locking mutex_req_file\n");

        if (filename != NULL)
            free(filename);

        // Lock for next iteration
        pthread_mutex_lock(&mutex_req_file);
    }

    // Unlock after last iteration
    pthread_mutex_unlock(&mutex_req_file);
}

void main(int argc, char *argv[]) {
    register int i;                 // to iterate in loops
    pthread_t *th;                  // dynamic array of thread identifiers
    struct sockaddr_in server_addr; // to store server IP-port

    signal(SIGINT, exit_successfully);
    process_arguments(argc, argv);

    // Open the request and log files
    // and the corresponding global variables will be used by the threads
    if ((req_file_handle = fopen(req_file, "r")) == NULL) {
        perror("Error: Could not open request file");
        exit(8);
    }

    if ((log_file_handle = fopen(log_file, "w")) == NULL) {
        perror("Error: Could not open log file");
        exit(9);
    }
    setbuf(log_file_handle, NULL); // Disable buffering for the log file

    // Dynamic memory allocation for the array of thread identifiers
    // TODO: TO BE COMPLETED BY STUDENT
    th = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);

    // Configure the structure with the server IP and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_srvwww);
    server_addr.sin_port = htons(srvwww_port);

    // Creation of download threads
    // TODO: TO BE COMPLETED BY STUDENT
    for (i = 0; i < nthreads; i++) {
        if (pthread_create(&th[i], NULL, (void *)download_thread,
                           &server_addr) != 0) {
            print_log("Error: Could not create thread\n");
            exit(11);
        }
    }

    // Wait for all threads to finish
    for (i = 0; i < nthreads; i++) {
        pthread_join(th[i], NULL);
    }
}
