#ifndef __INTERNAL__URING_REACTOR_H__
#define __INTERNAL__URING_REACTOR_H__

#include "uring_reactor/uring_reactor.h"
#include "liburing.h"
#include "map.h"

typedef struct {
  // configuration parameters for the reactor
  uring_reactor_config_t config;
  // This is the primary ring on which io operations will
  // be scheduled
  struct io_uring ring;
  // Ring file descriptor, will be set after initialization.
  int ring_fd;
  // A map of userdata -> waker for registered events. The
  // waker is owned by the reactor, so it should free them
  // once used.
  hashmap *waker_map;
  // A map of userdata -> result
  hashmap *result_map;
  // ID for the next task
  __u64 id;
} uring_reactor_t;

_Bool uring_reactor_t_react(reactor_impl *);
void uring_reactor_t_free(reactor_impl *);

_Bool uring_reactor_t_init(uring_reactor_t*, uring_reactor_config_t);
void uring_reactor_t_process_cqe(uring_reactor_t *, struct io_uring_cqe *);
struct io_uring_cqe *uring_reactor_t_consume_result(uring_reactor_t *, __u64);

__u64 uring_reactor_t_next_id(uring_reactor_t*);

#endif
