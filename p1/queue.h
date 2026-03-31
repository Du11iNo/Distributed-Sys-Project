#include <netinet/in.h> // Needed for sockaddr_in
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include "util.h"

typedef struct
{
    char msg[MSG_SIZE];             // request sent by the client
    int s;                          // communication socket with the client
    struct sockaddr_in client_addr; // client's IP address
    unsigned char is_stream;        // boolean. is it connection-oriented?
} queue_item;

struct Queue
{
    queue_item **items;     // the queue itself is an array of pointers to queue_item
    int head;
    int tail;
    int queue_size;
    pthread_mutex_t mutex_head;
    pthread_mutex_t mutex_tail;
    sem_t num_slots;
    sem_t num_used;
};
typedef struct Queue Queue;

void init_queue(Queue *queue, int queue_size);
void free_queue(Queue *queue);
void enqueue(Queue *queue, queue_item *item);
queue_item *dequeue(Queue *queue);
