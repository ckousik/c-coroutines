#include "simple_thread_executor.h"
#include "log.h"
#include "queue.h"
#include "task.h"

#include <stdlib.h>

typedef struct {
  queue_t *ready_queue;
  size_t pending_count;
} simple_thread_executor_t;

// simple waker
typedef struct {
  task_t *task;
  simple_thread_executor_t *executor;
} simple_thread_executor_waker_t;

// waker functions
// wake a task
void simple_thread_executor_waker_t_wake(const waker_impl *waker) {
  simple_thread_executor_waker_t *impl =
      (simple_thread_executor_waker_t *)waker;
  log_debug("WAKER: %lu", waker);
  if (impl->executor == NULL || impl->task == NULL) {
    return;
  }
  simple_thread_executor_t *executor = impl->executor;
  if (executor->pending_count == 0) {
    // ERROR condition
    log_error("impl called when no task if pending");
    return;
  }
  // decrement the pending count
  executor->pending_count--;
  // push task to ready queue
  log_debug("PUSHING TASK TO READY QUEUE: %lu", impl->task);
  if (!queue_t_push_back(executor->ready_queue, impl->task)) {
    // ERROR condition
    log_error("task queue is full");
    return;
  }
}

// free the waker
void simple_thread_executor_waker_t_free(waker_impl *waker) {
  simple_thread_executor_waker_t *impl =
      (simple_thread_executor_waker_t *)waker;
  free(impl);
}

// executor

// create a new waker
waker_t *
simple_thread_executor_t_create_waker(simple_thread_executor_t *executor,
                                      task_t *task) {
  simple_thread_executor_waker_t *waker = NULL;
  waker_t *result = NULL;

  if ((waker = (simple_thread_executor_waker_t *)malloc(sizeof(*waker))) ==
      NULL) {
    return NULL;
  }
  *waker = (simple_thread_executor_waker_t) {
    .executor = executor,
    .task = task,
  };
  result = waker_t_alloc(waker, simple_thread_executor_waker_t_wake,
                         simple_thread_executor_waker_t_free);
  if (result == NULL) {
    // free waker allocation
    free(waker);
  }
  return result;
}

// spawn a new task
int simple_thread_executor_t_spawn(executor_impl *impl, task_t *task) {
  log_debug("pushing new task");
  simple_thread_executor_t *executor = (simple_thread_executor_t *)impl;
  if (queue_t_push_back(executor->ready_queue, task)) {
    return 0;
  }
  log_warn("failed to push task to queue");
  return 1;
}

void simple_thread_executor_t_run(executor_impl *impl) {
  simple_thread_executor_t *executor = (simple_thread_executor_t *)impl;
  // Poll at most `len` tasks from the queue. Polled tasks could push
  // more tasks onto the queue which will be polled in subsequent invokations
  // to poll.
  size_t queue_size = queue_t_len(executor->ready_queue);
  size_t idx = 0;
  for (; idx < queue_size; idx++) {
    task_t *task = NULL;
    if (!queue_t_pop_front(executor->ready_queue, (item_t **)&task)) {
      // ERROR condition
      log_error("inconsistent queue size");
      return;
    }
    // ownership of this waker is moved to the task
    waker_t *waker = NULL;
    if ((waker = simple_thread_executor_t_create_waker(executor, task)) ==
        NULL) {
      // ERROR condition. Could not allocate waker
      log_error("could not allocate waker");
      continue;
    }
    task_state_t state = task_t_poll(task, waker);
    if (state == READY) {
      queue_t_push_back(executor->ready_queue, task);
      continue;
    }
    if (state == PENDING) {
      // increment pending count
      executor->pending_count++;
      continue;
    }
  }
}

_Bool simple_thread_executor_t_has_tasks(executor_impl *impl) {
  simple_thread_executor_t *executor = (simple_thread_executor_t *)impl;
  return executor->pending_count + queue_t_len(executor->ready_queue) > 0;
}

void simple_thread_executor_t_free(executor_impl *impl) {
  simple_thread_executor_t *executor = (simple_thread_executor_t *)impl;
  queue_t_free(executor->ready_queue, (void (*)(void *))task_t_free);
  free(executor);
}

const size_t DEFAULT_EXECUTION_QUEUE_LENGTH = 2048;

executor_t *simple_thread_executor_t_create(void) {
  simple_thread_executor_t *executor = NULL;
  executor_t *result = NULL;

  if ((executor = malloc(sizeof(*executor))) == NULL) {
    return NULL;
  }
  // allocate the ready queue
  executor->ready_queue = queue_t_create(DEFAULT_EXECUTION_QUEUE_LENGTH);
  if (executor->ready_queue == NULL) {
    free(executor);
    return NULL;
  }

  result = executor_t_alloc(
      executor, simple_thread_executor_t_spawn, simple_thread_executor_t_run,
      simple_thread_executor_t_has_tasks, simple_thread_executor_t_free);
  // free the executor if the vtable failed
  if (result == NULL) {
    simple_thread_executor_t_free(executor);
  }
  return result;
}