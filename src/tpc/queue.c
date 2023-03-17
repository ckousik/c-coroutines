#include "queue.h"
#include "log.h"

#include <stdlib.h>

// ring buffer
struct queue_t {
  void **raw;
  size_t capacity, len;
  size_t head, tail;
};

queue_t *queue_t_create(size_t capacity) {
  queue_t *queue = malloc(sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  void **raw = (void **)malloc(sizeof(void *) * capacity);
  if (raw == NULL) {
    log_warn("could not allocate queue");
    free(queue);
    return NULL;
  }
  *queue = (queue_t){
      .raw = raw, .capacity = capacity, .len = 0, .head = 0, .tail = 0};
  return queue;
}
// push an item onto the queue
_Bool queue_t_push_back(queue_t *queue, item_t *item) {
  if (queue->len == queue->capacity) {
    return 0;
  }
  // array base
  queue->raw[queue->tail] = item;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->len++;
  return 1;
}

// pop an item from the queue
_Bool queue_t_pop_front(queue_t *queue, item_t **item) {
  if (queue->len == 0) {
    return 0;
  }
  *item = queue->raw[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  queue->len--;
  return 1;
}

void queue_t_free(queue_t *queue, void (*free_item)(item_t *)) {
  size_t idx = 0;
  if (free_item != NULL) {
    for (; idx < queue->len; idx++) {
      free_item(queue->raw[idx]);
    }
  }
  free(queue->raw);
}

// length of the queue
inline size_t queue_t_len(queue_t *queue) { return queue->len; }