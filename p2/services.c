#include "srvwww.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// This external variable is assigned in main, through the code
// injected by the Python script.
// Its value is used by the different services implemented here.
extern char *documentroot;

void free_list(chunk_list *l) {
    // Function that frees the memory of each node in the linked list
    // of chunks, which is used in the getchunk_1_svc service.
    // It will be called from that service to free the memory
    // allocated during the previous execution of the same service.

    chunk_list *aux;
    while (l != NULL) {
        aux = l;
        l = l->next;
        free(aux->chunk_data.chunk_val); // Free memory of the chunk
        free(aux);
    }
}

// Function that returns the size of a file in bytes
long get_file_size(const char *filename) {
    struct stat st;

    st.st_size = -1; // A file cannot have a negative size
    if (stat(filename, &st) == -1) {
        fprintf(stderr, "Error obtaining size of file %s: %s\n", filename,
                strerror(errno));
        return -1;
    }
    return st.st_size;
}

Result *existfile_1_svc(filename_string *fname, struct svc_req *request) {
    char filename[DIR_SIZE]; // To contain the filename including documentroot
    static Result res;
    struct stat buf; // For the stat() call

    // Check whether the file documentroot/fname exists
    // and return the appropriate result in res
    // TODO: TO BE COMPLETED BY STUDENT

    sprintf(filename, "%s/%s", documentroot, *fname);

    if (stat(filename, &buf) != 0) {
        res.case_id = 0;
        res.Result_u.b = 0;
    } else {
        res.case_id = 0;
        res.Result_u.b = 1;
    }

    return (&res);
}

Result *getsize_1_svc(filename_string *fname, struct svc_req *request) {
    char filename[DIR_SIZE]; // To contain the filename including documentroot
    static Result res;
    long size;

    // Open documentroot/fname and obtain its size,
    // returning the appropriate result in res
    // TODO: TO BE COMPLETED BY STUDENT

    sprintf(filename, "%s/%s", documentroot, *fname);
    size = get_file_size(filename);

    if (size < 0) {
        res.case_id = 3;
        res.Result_u.err = "Error: Missing file\n";
    } else {
        res.case_id = 1;
        res.Result_u.val = size;
    }

    return &res;
}

void error_and_close_file(char *msg, int fd, Result *res) {
    // Auxiliary utility function that prepares the Result structure
    // with an error message, and closes file descriptor fd
    // TODO: TO BE COMPLETED BY STUDENT

    close(fd);
    res->case_id = 3;
    res->Result_u.err = msg;
}

Result *getchunk_1_svc(param1 *fileitem, struct svc_req *request) {
    char filename[DIR_SIZE]; // To contain the filename including documentroot
    int fd; // File descriptor from which we will read the chunk
    char chunk_buf[CHUNK_LEN]; // Buffer to store the chunk read
    static Result res;
    long offset;         // Offset from which the file will be read
    int remaining;       // Number of bytes left to read
    int chunk_size;      // Size of the chunk read
    int read_bytes;      // Number of bytes read in each iteration
    int accumulated = 0; // Accumulated number of bytes read

    // Auxiliary variables for building the linked list
    static chunk_list *first =
        NULL;                // Pointer to the first node of the linked list
    chunk_list *last = NULL; // Pointer to the last node of the linked list
    chunk_list *aux = NULL;  // Auxiliary pointer

    // Free memory from any previously allocated chunk list
    free_list(first);
    first = NULL;

    // Open documentroot/fname
    sprintf(filename, "%s/%s", documentroot, fileitem->filename);
    fprintf(stderr,
            "Server: Client requests a chunk of file %s starting at offset %ld "
            "with size %d bytes\n",
            filename, fileitem->offset, fileitem->size);

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        error_and_close_file("Error opening file to obtain a chunk\n", fd,
                             &res);
        return (&res);
    }

    // Move file pointer to the indicated offset
    offset = fileitem->offset;
    remaining = fileitem->size; // Number of bytes to read

    // Basic checks: offset and remaining cannot be negative
    // TODO: TO BE COMPLETED BY STUDENT

    if (offset < 0 || remaining < 0) {
        error_and_close_file("Error: Offset or Remaining cannot be negative",
                             fd, &res);
        return &res;
    }

    if (lseek(fd, offset, SEEK_SET) < 0) {
        error_and_close_file(
            "Error moving file pointer to the indicated offset\n", fd, &res);
        return (&res);
    }

    fprintf(stderr, "Server: Positioned in file %s starting at offset %ld\n",
            filename, fileitem->offset);

    // Read the file in blocks of at most CHUNK_LEN bytes
    // and append them to the linked list of chunks.
    // If EOF is reached, exit the loop.
    chunk_size = min(CHUNK_LEN, remaining);
    fprintf(stderr,
            "Server: Chunk size to read from file %s at offset %ld is %d\n",
            filename, fileitem->offset, chunk_size);

    accumulated = 0;
    explicit_bzero(chunk_buf, chunk_size);

    while ((chunk_size > 0) &&
           (read_bytes = read(fd, chunk_buf, chunk_size)) > 0) {
        if (read_bytes < 0) {
            error_and_close_file("Error reading file\n", fd, &res);
            if (first != NULL)
                free_list(first);
            first = NULL;
            return (&res);
        }

        fprintf(stderr, "Server: Read %d bytes from file %s\n", read_bytes,
                filename);

        if (read_bytes < chunk_size) {
            chunk_size = read_bytes;
        }

        accumulated += read_bytes;
        remaining -= read_bytes;

        // First chunk → create first node
        if (first == NULL) {
            first = (chunk_list *)malloc(sizeof(chunk_list));
            if (first == NULL) {
                error_and_close_file(
                    "Error allocating memory for first list node\n", fd, &res);
                return (&res);
            }

            // Allocate memory for the chunk
            // TODO: TO BE COMPLETED BY STUDENT

            first->chunk_data.chunk_val = (char *)malloc(read_bytes);

            if (first->chunk_data.chunk_val == NULL) {
                error_and_close_file("Error allocating memory for chunk\n", fd,
                                     &res);
                return (&res);
            }

            memmove(first->chunk_data.chunk_val, chunk_buf, chunk_size);
            first->chunk_data.chunk_len = chunk_size;
            first->next = NULL;
            last = first;
        } else {
            // Subsequent chunks → append to list
            aux = (chunk_list *)malloc(sizeof(chunk_list));
            if (aux == NULL) {
                error_and_close_file("Error allocating memory for list node\n",
                                     fd, &res);
                free_list(first);
                first = NULL;
                return (&res);
            }

            // Allocate memory for the chunk
            // TODO: TO BE COMPLETED BY STUDENT

            aux->chunk_data.chunk_val = (char *)malloc(read_bytes);

            if (aux->chunk_data.chunk_val == NULL) {
                error_and_close_file("Error allocating memory for chunk\n", fd,
                                     &res);
                free_list(first);
                first = NULL;
                return (&res);
            }

            memcpy(aux->chunk_data.chunk_val, chunk_buf, chunk_size);
            aux->chunk_data.chunk_len = chunk_size;
            aux->next = NULL;

            if (last)
                last->next = aux;
            last = aux;
        }

        chunk_size = min(CHUNK_LEN, remaining);
        bzero(chunk_buf, chunk_size);
    }

    if (accumulated != fileitem->size) {
        fprintf(stderr,
                "Server: In file %s at offset %ld expected to read %d bytes "
                "but read %d\n",
                filename, fileitem->offset, fileitem->size, accumulated);

        error_and_close_file("Error: Not all required bytes were read\n", fd,
                             &res);
        free_list(first);
        first = NULL;
        return (&res);
    }

    close(fd);

    // TODO: TO BE COMPLETED BY STUDENT

    res.case_id = 2;
    res.Result_u.list = first;

    return (&res);
}
