#ifndef __TPC_QUEUE_H
#define __TPC_QUEUE_H

#include <stddef.h>

typedef struct queue_t queue_t;
typedef void item_t;

// create a new queue
queue_t *queue_t_create(size_t capacity);

// free
void queue_t_free(queue_t *queue, void (*free_item)(item_t *));

// number of elements in the queue
size_t queue_t_len(queue_t *);

// Pushes a new item onto the queue. Returns
// false if the queue is full.
_Bool queue_t_push_back(queue_t *, item_t *);

// Pops an element from the queue. Returns false
// if the queue is empty.
_Bool queue_t_pop_front(queue_t *, item_t **);

#endif