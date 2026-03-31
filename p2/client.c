/*
  RPC client that simulates the operations of several clients of the
  www server
*/
#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "srvwww.h"
#include "util.h"

#define MAX_CLIENT_THREADS 10

// Global variables

// IP of the srvwww process
char *ip_srv = NULL;

// number of clients
int num_clients;

// maximum size of the chunk to download in each remote invocation
int max_chunk_size;

// data type that client threads will receive
struct thread_data
{
    unsigned char client_id;
    FILE *fp_in;
};

typedef struct thread_data thread_data;

pthread_mutex_t m;      // mutex to serialize remote invocations
pthread_mutex_t mpet;   // mutex for mutual exclusion on the request file

void write_chunk_list(chunk_list *l, int fd)
{
    // Function that writes the chunks of the linked list into the file fd
    chunk_list *aux = l;
    while (aux != NULL)
    {
        write(fd, aux->chunk_data.chunk_val, aux->chunk_data.chunk_len); // Write chunk to file
        aux = aux->next;
    }
}

void download_file(int client_id, CLIENT *cl, char *filename, int max_chunk_size)
{
    Result *res;   // Result returned by the RPCs
    param1 q1;     // Parameter for RPCs that use type param1
    long file_size; // Size of the file to download
    long offset;    // Offset from which the file will be downloaded
    int fd_out;     // Output file descriptor where the file will be stored

    // TODO: TO BE COMPLETED BY STUDENT
    ...
    ...

    if (...)   // TODO: TO BE COMPLETED BY STUDENT
    {
        // If the file exists, continue by asking for its size
        fprintf(stderr, "Client: %d File %s exists on the server.\n", client_id, filename);
        // TODO: TO BE COMPLETED BY STUDENT
        ...
        ...

        if (res->case_id == 1 && res->Result_u.val >= 0)
        {
            file_size = res->Result_u.val; // Size of the file to download
            fprintf(stderr, "Client: %d File %s has a size of %ld bytes.\n",
                    client_id, filename, res->Result_u.val);

            // If the file size is positive, proceed to download it
            // Create the output file with the same name as the file to download
            if ((fd_out = open(filename, O_WRONLY | O_CREAT, 0644)) < 0)
            {
                // If the output file could not be opened, report and continue with next file
                fprintf(stderr, "Client: %d Could not open output file %s for writing.\n",
                        client_id, filename);
                free(filename);
                filename = NULL;
            }
            else
            {
                fprintf(stderr, "Client: %d Opens output file %s for writing.\n",
                        client_id, filename);

                // Download the file in chunks of size max_chunk_size
                offset = 0; // Initial offset
                while (offset < file_size)
                {
                    // Prepare parameter for the RPC
                    q1.filename = filename;
                    q1.offset = offset;
                    q1.size = (max_chunk_size < (file_size - offset)) ?
                               max_chunk_size : (file_size - offset);

                    fprintf(stderr,
                            "Client: %d Requests a chunk of file %s from offset %ld with size %d bytes\n",
                            client_id, filename, q1.offset, q1.size);

                    res = getchunk_1(&q1, cl);
                    if (res == NULL)
                    {
                        clnt_perror(cl, "Error in remote invocation getchunk_1");
                        fprintf(stderr,
                                "Client: %d Error invoking RPC getchunk_1 for file %s\n",
                                client_id, filename);
                        close(fd_out);
                        free(filename);
                        filename = NULL;
                        break;
                    }
                    else if (res->case_id == 2) // RPC completed successfully
                    {
                        fprintf(stderr,
                                "Client: %d Receives a chunk of file %s of size %d bytes\n",
                                client_id, filename, q1.size);

                        // Write the chunk to the output file
                        write_chunk_list(res->Result_u.list, fd_out);

                        // Free memory of the chunk list
                        xdr_free((xdrproc_t) xdr_chunk_list, (char *)&res->Result_u.list);

                        offset += q1.size; // Update offset
                    }
                    else if (res->case_id == 3) // Error in RPC
                    {
                        fprintf(stderr,
                                "Client: %d Error downloading chunk of file %s: %s\n",
                                client_id, filename, res->Result_u.err);
                        free(filename);
                        filename = NULL;
                        break;
                    }
                }

                close(fd_out);
                fprintf(stderr, "Client: %d Finished downloading file %s.\n",
                        client_id, filename);
            }

        }
        else // Negative file size or RPC error
        {
            if (res->case_id == 3)
            {
                fprintf(stderr, "Client: %d receives error message: %s\n",
                        client_id, res->Result_u.err);
            }
            else
            {
                fprintf(stderr,
                        "Client: %d Tries to download file %s which server reports as having negative size.\n",
                        client_id, filename);
            }
        }
    }
    else // File does not exist or RPC error
    {
        if (res->case_id == 3)
        {
            fprintf(stderr, "Client: %d receives error message: %s\n",
                    client_id, res->Result_u.err);
        }
        else
        {
            fprintf(stderr,
                    "Client: %d Tries to download file %s which does not exist on the server.\n",
                    client_id, filename);
        }
    }
}

void process_tags(int client_id, CLIENT *cl, char *filename, int max_chunk_size)
{
    /*
        This function downloads the images referenced inside the file
        specified in the parameter 'filename'. The images will be downloaded
        using RPC calls to the server.
    */

    FILE *fp_in;             // to read the input file
    char line[LINE_SIZE];    // to store each line read from the file
    char *pos;
    char *s;                 // for use with strtok_r
    char *token;

    // Prepare request to obtain the file size and send it
    // Open the file for reading

    if ((fp_in = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr,
                "Client %d Error: Could not open file %s for reading\n",
                client_id, filename);
        exit(18);
    }

    // Read the file line by line
    while (fgets(line, LINE_SIZE, fp_in) != NULL)
    {
        pos = strstr(line, "<img ");
        if (pos == NULL)
        {
            pos = strstr(line, "<IMG ");
        }

        if (pos != NULL)  // there is an image reference inside the line
        {
            pos += 5; // skip "<img " or "<IMG "
            token = NULL;

            // extract the string until the end of the tag
            token = strtok_r(pos, ">\n\r ", &s);
            if (token != NULL)
            {
                download_file(client_id, cl, token, max_chunk_size);
            }
        }
    }

    fclose(fp_in);
}

void *process_downloads(thread_data *p)
{
    CLIENT *cl;             // CLIENT structure for RPC
    FILE *req_file_handle;  // File from which download requests are read
    int client_id;          // Client number (for debug messages)
    char buffer[LINE_SIZE]; // Buffer to read one line from the request file
    char msg[LINE_SIZE];    // Buffer for error messages sent to log_debug()
    char *token = NULL;     // For tokenizing lines from the request file
    char *loc;              // For use with strtok_r
    char *filename = NULL;  // Name of the file to download

    client_id = p->client_id;     // Capture client ID in a local variable
    req_file_handle = p->fp_in;   // Capture pointer to request file
    free(p);                      // Parameter no longer needed

    bzero(buffer, LINE_SIZE);

    pthread_mutex_lock(&mpet); // Lock mutex protecting the request file
    while (fgets(buffer, LINE_SIZE, req_file_handle) != NULL)
    {
        fprintf(stderr, "Client: %d reads WWW request: %s\n", client_id, buffer);
        pthread_mutex_unlock(&mpet); // Unlock mutex protecting the request file

        // Tokenize the line read
        loc = NULL;
        token = strtok_r(buffer, " \n", &loc);
        if (token == NULL)
        {
            sprintf(msg, "Client: %d Error extracting first token from WWW request\n", client_id);
            log_debug(msg);
            exit(12);
        }

        // First token is the filename
        filename = strdup(token);
        if (filename == NULL)
        {
            sprintf(msg, "Error: Could not allocate memory to store the filename to download.\n");
            exit(15);
        }
        fprintf(stderr, "Client: %d The file to download is %s\n", client_id, filename);

        // Second token is the protocol to use (T or U)
        token = strtok_r(NULL, "\n", &loc);

        // Initialize CLIENT structure according to chosen protocol
        if (strcmp(token, "T") == 0)
        {
            fprintf(stderr, "Client: %d File %s will be downloaded using TCP protocol\n",
                    client_id, filename);
            // TODO: TO BE COMPLETED BY STUDENT
            ...
            ...
        }
        else if (strcmp(token, "U") == 0)
        {
            fprintf(stderr, "Client: %d File %s will be downloaded using UDP protocol\n",
                    client_id, filename);
            // TODO: TO BE COMPLETED BY STUDENT
            ...
            ...
        }
        else // Unknown protocol
        {
            fprintf(stderr, "Client: %d protocol for downloading file %s is unknown\n",
                    client_id, filename);
            exit(45);
        }

        // Check for possible error in clnt_create
        if (cl == NULL)
        {
            clnt_pcreateerror("Error in clnt_create in Client thread");
            exit(46);
        }

        // Under mutex protection, perform all remote invocations
        // needed to download this file to avoid race conditions
        pthread_mutex_lock(&m);

        // First check whether the file exists
        // TODO: TO BE COMPLETED BY STUDENT
        ...
        ...

        process_tags(client_id, cl, filename, max_chunk_size);

        // Finished all RPCs needed for this file, release lock
        pthread_mutex_unlock(&m);

        // Free CLIENT structure and filename
        clnt_destroy(cl);
        if (filename != NULL)
        {
            free(filename);
            filename = NULL;
        }

        bzero(buffer, LINE_SIZE);

        // Before next iteration, lock mutex again
        pthread_mutex_lock(&mpet);
    }

    // Finished reading request file, release mutex
    pthread_mutex_unlock(&mpet);
    return NULL;
}

int main(int argc, char *argv[])
{
    int i;               // Loop index
    pthread_t *th;       // dynamic array of thread references
    thread_data *q;      // pointer to structure passed to threads
    FILE *fp;            // pointer to the opened request file

    if (argc != 5)
    {
        fprintf(stderr,
                "Usage: %s <num_clients> <ip_rpc_server> <downloads_file> <max_chunk_size>\n",
                argv[0]);
        exit(1);
    }

    // Validate each argument and assign to the appropriate variable
    if (!validate_number(argv[1]))
    {
        fprintf(stderr, "Parameter <num_clients> is not a number.\n");
        exit(2);
    }

    num_clients = atoi(argv[1]);
    if ((num_clients < 1) || (num_clients > 10))
    {
        fprintf(stderr, "Parameter <num_clients> must be >= 1 and <= 10\n");
        exit(3);
    }

    ip_srv = strdup(argv[2]);
    if (!validate_ip(argv[2]))
    {
        fprintf(stderr, "The IP provided is not valid\n");
        exit(4);
    }

    // Try to open the file to detect problems early (main does not use this file,
    // it will be passed to the client threads)
    if ((fp = fopen(argv[3], "r")) == NULL)
    {
        fprintf(stderr, "Cannot open the file containing the WWW requests\n");
        exit(5);
    }

    if (!validate_number(argv[4]))
    {
        fprintf(stderr, "Parameter <max_chunk_size> is not a number.\n");
        exit(6);
    }

    max_chunk_size = atoi(argv[4]);
    if (max_chunk_size <= 0)
    {
        fprintf(stderr, "Parameter <max_chunk_size> must be a positive number.\n");
        exit(7);
    }

    // Since the client stubs share a static variable, to avoid threads
    // interfering with each other, a mutex is used to prevent two threads
    // from making the same RPC simultaneously
    // TODO: TO BE COMPLETED BY STUDENT
    ...
    ...

    // Another mutex is needed to prevent two threads from reading
    // the request file at the same time
    // TODO: TO BE COMPLETED BY STUDENT
    ...
    ...

    // Allocate space for the dynamic array of thread references
    th = (pthread_t *)malloc(sizeof(pthread_t) * num_clients);
    if (th == NULL)
    {
        fprintf(stderr, "Could not allocate space for thread references.\n");
        exit(10);
    }

    // Launch threads
    // TODO: TO BE COMPLETED BY STUDENT
    ...
    ...

    // Wait for all threads to finish
    // TODO: TO BE COMPLETED BY STUDENT
    ...
    ...

    fclose(fp);
    exit(0);
}

