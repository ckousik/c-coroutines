#include "internal/uring_reactor.h"
#include "liburing.h"
#include "tpc/log.h"
#include "map.h"
#include "tpc/reactor.h"
#include "tpc/waker.h"

#include <stdlib.h>
#include <string.h>

reactor_t *uring_reactor_t_create(uring_reactor_config_t config) {
  uring_reactor_t* reactor = (uring_reactor_t *)malloc(sizeof(uring_reactor_t));
  if (reactor == NULL) {
    return NULL;
  }
  if (!uring_reactor_t_init(reactor, config)) {
    log_debug("free reactor");
    free(reactor);
  }
  (reactor->id) = 0;
  __auto_type result =
      reactor_t_alloc(reactor, uring_reactor_t_react, uring_reactor_t_free);
  if (result == NULL) {
    free(reactor);
  }
  return result;
}

// initialize a new reactor
_Bool uring_reactor_t_init(uring_reactor_t *reactor,
                           uring_reactor_config_t config) {
  if ((reactor->ring_fd =
           io_uring_queue_init(config.entries, &(reactor->ring), 0)) < 0) {
    return 0;
  }
  reactor->waker_map = hashmap_create();
  reactor->result_map = hashmap_create();
  return 1;
}

void uring_reactor_t_process_cqe(uring_reactor_t *reactor,
                                 struct io_uring_cqe *cqe) {
  // Do nothing with an empty cqe
  if (cqe == NULL) {
    return;
  }
  // find the associated waker
  log_debug("COMPLETION ID: %d", cqe->user_data);
  uintptr_t waker;
  if (hashmap_get(reactor->waker_map, (const void*) &cqe->user_data,
                  sizeof(__u64), &waker)) {
    // store the associated result
    __auto_type result_map = reactor->result_map;
    // clone the cqe
    struct io_uring_cqe *result_cqe = NULL;
    if ((result_cqe = (struct io_uring_cqe *)malloc(sizeof(*result_cqe))) ==
        NULL) {
      // return could not allocate result
      log_error("could not allocate result");
      return;
    }
    if (memcpy(result_cqe, cqe, sizeof(*cqe)) == NULL) {
      free(result_cqe);
      log_error("could not copy result");
      return;
    }
    hashmap_set(result_map, (const void *)&(result_cqe->user_data),
                sizeof(__u64), (uintptr_t)result_cqe);
    log_debug("FETCHED WAKER: %llu", (waker_t*)waker);
    // wake the associated task
    waker_t_wake((waker_t *)waker);
    // free the waker
    waker_t_free((waker_t *)waker);
    // unset the waker
    hashmap_remove(reactor->waker_map, (const void *) &cqe->user_data, sizeof(__u64));
  }
}

struct io_uring_cqe *uring_reactor_t_consume_result(uring_reactor_t *reactor,
                                                    __u64 key) {
  uintptr_t result = 0;
  if (hashmap_get(reactor->result_map, (const void *)&key, sizeof(key),
                  &result)) {
    // delete the hashmap entry so it cannot be double-freed
    hashmap_remove(reactor->result_map, (const void *)&key, sizeof(key));
  }
  return (struct io_uring_cqe *)result;
}

// non-blocking react
_Bool uring_reactor_t_react(reactor_impl *impl) {
  // reactor
  uring_reactor_t *reactor = (uring_reactor_t *)impl;
  // array of cqe pointers to peek
  struct io_uring_cqe *cqes[10];
  // peek into the ring
  __auto_type peeked = io_uring_peek_batch_cqe(&(reactor->ring), cqes, 10);

  if (peeked == 0) {
    return 1;
  }

  if (peeked < 0) {
    log_error("error when peeking from ring: %d", peeked);
    return 0;
  }
  // do something with the results, like waking up tasks.
  for (int idx = 0; idx < peeked; idx++) {
    struct io_uring_cqe *cqe = cqes[idx];
    uring_reactor_t_process_cqe(reactor, cqe);
  }
  // advance the ring
  for (int idx = 0; idx < peeked; idx++) {
    struct io_uring_cqe *cqe = cqes[idx];
    io_uring_cqe_seen(&(reactor->ring), cqe);
  }
  return 1;
}

void uring_reactor_t_wake_pending(void *k, size_t ks, uintptr_t value,
                                  void *usr) {
  __auto_type waker = (waker_t *)value;
  waker_t_wake(waker);
}

void uring_reactor_t_free_entry(void *k, size_t ks, uintptr_t value,
                                void *usr) {
  free((void *)value);
}

void uring_reactor_t_free(reactor_impl *impl) {
  // reactor
  uring_reactor_t *reactor = (uring_reactor_t *)impl;
  io_uring_queue_exit(&(reactor->ring));
  // wake any pending routines
  hashmap_iterate(reactor->waker_map, uring_reactor_t_wake_pending, NULL);
  // free wakers
  hashmap_iterate(reactor->waker_map, uring_reactor_t_free_entry, NULL);
  // free results
  hashmap_iterate(reactor->result_map, uring_reactor_t_free_entry, NULL);
}

__u64 uring_reactor_t_next_id(uring_reactor_t *reactor) {
  __auto_type result = reactor->id;
  log_debug("ID: %d", result);
  reactor->id++;
  return result;
}
