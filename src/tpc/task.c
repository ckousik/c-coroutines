#include "task.h"
#include <stdlib.h>

struct task_t {
  task_impl *ptr;
  task_state_t (*poll)(const task_impl *, waker_t *);
  task_state_t (*state)(const task_impl *);
  const void *(*output)(const task_impl *);
  void (*free)(task_impl *);
};

task_t *alloc_raw_task(task_impl *ptr,
                       task_state_t (*poll)(const task_impl *, waker_t *),
                       task_state_t (*state)(const task_impl *),
                       const void *(*output)(const task_impl *),
                       void (*free)(task_impl *)) {
  struct task_t *task = (struct task_t *)malloc(sizeof(struct task_t));
  (*task) = (struct task_t){
      .ptr = ptr,
      .poll = poll,
      .state = state,
      .output = output,
      .free = free,
  };
  return task;
}

task_state_t task_t_poll(task_t *task, waker_t *waker) {
  return task->poll(task->ptr, waker);
}

const void *task_t_output(struct task_t *task) {
  return task->output(task->ptr);
}

task_state_t task_t_state(task_t *task) { return task->state(task->ptr); }

void task_t_free(struct task_t *task) {
  task->free(task->ptr);
  free(task);
}
