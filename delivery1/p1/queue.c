#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void init_queue(Queue *queue, int queue_size) {
    register int i;

    if (queue_size < 1) {
        perror("Error: The queue size must be greater than or equal to 1");
        exit(1);
    }
    if (queue == NULL) {
        perror("Error: The pointer to the queue is NULL in init_queue");
        exit(2);
    }
    // TODO: TO BE COMPLETED BY STUDENT

    queue->items = (queue_item **)calloc(queue_size, sizeof(queue_item *));
    queue->head = 0;
    queue->tail = 0;
    queue->queue_size = queue_size;

    pthread_mutex_init(&queue->mutex_head, NULL);
    pthread_mutex_init(&queue->mutex_tail, NULL);

    sem_init(&queue->num_slots, 0, queue_size);
    sem_init(&queue->num_used, 0, 0);
}

void free_queue(Queue *queue) {
    // Memory pointed to by each pointer stored in the queue must be freed,
    // as well as the memory where those pointers are stored,
    // and the semaphores and mutexes must be destroyed.

    // TODO: TO BE COMPLETED BY STUDENT

    register int i;

    for (i = 0; i < queue->queue_size; i++) {
        free(queue->items[i]);
    }
    free(queue->items);
    pthread_mutex_destroy(&queue->mutex_head);
    pthread_mutex_destroy(&queue->mutex_tail);
    sem_destroy(&queue->num_slots);
    sem_destroy(&queue->num_used);

    free(queue);
}

void enqueue(Queue *queue, queue_item *item) {
    // copy the pointer directly into the appropriate position in the queue
    // TODO: TO BE COMPLETED BY STUDENT

    sem_wait(&queue->num_slots);
    pthread_mutex_lock(&queue->mutex_head);

    queue->items[queue->head] = item;

    queue->head = (queue->head + 1) % queue->queue_size;

    pthread_mutex_unlock(&queue->mutex_head);
    sem_post(&queue->num_used);
}

queue_item *dequeue(Queue *queue) {
    queue_item *p;

    // TODO: TO BE COMPLETED BY STUDENT

    sem_wait(&queue->num_used);
    pthread_mutex_lock(&queue->mutex_tail);

    p = queue->items[queue->tail];
    queue->items[queue->tail] = NULL;

    queue->tail = (queue->tail + 1) % queue->queue_size;

    pthread_mutex_unlock(&queue->mutex_tail);
    sem_post(&queue->num_slots);

    return p;
}
