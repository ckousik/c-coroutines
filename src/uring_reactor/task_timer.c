#include "internal/uring_reactor.h"
#include "log.h"
#include "task.h"

#include <stdlib.h>
#include <stdio.h>

// Creates a task that resolves when a timeout occurs
typedef struct {
  uring_reactor_t *reactor;
  struct __kernel_timespec ts;
  __u64 id;
  int state;
} uring_timer_task_t;

task_state_t uring_timer_task_t_poll(const task_impl *impl, waker_t *waker) {
  __auto_type task = (uring_timer_task_t *)impl;
  struct io_uring *ring = &(task->reactor->ring);
  if (task->state == 0) {
    __auto_type sqe = io_uring_get_sqe(ring);
    __auto_type waker_map = task->reactor->waker_map;

    io_uring_prep_timeout(sqe, &task->ts, 0, 0);
    sqe->user_data = task->id;
    io_uring_submit(ring);

    // register waker
    hashmap_set(waker_map, (const void *)&(task->id), sizeof(__u64),
                (uintptr_t)(void*)waker);
    log_debug("REGISTERED WAKER: %lu", (uintptr_t)(void*)waker);
    task->state++;
    return PENDING;
  }

  log_debug("TIMER WOKEN");

  // if woken, look for the result
  __auto_type result = uring_reactor_t_consume_result(task->reactor, task->id);
  if (result == NULL) {
    log_error("TIMER FAILED");
  }
  free(result);
  waker_t_free(waker);
  task->state++;
  return COMPLETED;
}

task_state_t uring_timer_task_t_state(const task_impl *impl) {
  __auto_type task = (uring_timer_task_t *)impl;
  switch (task->state) {
  case 0:
    return READY;
  case 1:
    return PENDING;
  default:
    return COMPLETED;
  }
}

const void *uring_timer_task_t_output(const task_impl *impl) { return NULL; }

void uring_timer_task_t_free(task_impl *impl) { free(impl); }

task_t *uring_timer_task_t_create(reactor_t *ireactor,
                                  struct __kernel_timespec ts) {
  __auto_type reactor = (uring_reactor_t*) reactor_t_impl(ireactor);
  __auto_type task = (uring_timer_task_t *)malloc(sizeof(uring_timer_task_t));
  if (task == NULL) {
    return NULL;
  }
  *task = (uring_timer_task_t){
      .reactor = reactor,
      .ts = ts,
      .id = uring_reactor_t_next_id(reactor),
      .state = 0,
  };
  return alloc_raw_task(task, uring_timer_task_t_poll, uring_timer_task_t_state,
                        uring_timer_task_t_output, uring_timer_task_t_free);
}