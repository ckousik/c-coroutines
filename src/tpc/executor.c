#include "executor.h"
#include "waker.h"

#include <stdlib.h>

struct executor_t {
  executor_impl *impl;
  int (*spawn)(executor_impl *, task_t *);
  void (*run)(executor_impl *);
  _Bool (*has_tasks)(executor_impl *);
  void (*free)(executor_impl *);
};

inline int executor_t_spawn(executor_t *executor, task_t *task) {
  return executor->spawn(executor->impl, task);
}

inline void executor_t_run(executor_t *executor) {
  return executor->run(executor->impl);
}

inline void executor_t_free(executor_t *executor) {
  // free the underlying implementation
  executor->free(executor->impl);
  // free the executor vtable
  free(executor);
}

inline _Bool executor_t_has_tasks(executor_t *executor) {
  return executor->has_tasks(executor->impl);
}

executor_t *executor_t_alloc(executor_impl *ptr,
                             int (*spawn)(executor_impl *, task_t *),
                             void (*run)(executor_impl *),
                             _Bool (*has_tasks)(executor_impl *),
                             void (*free)(executor_impl *)) {
  executor_t *executor = NULL;
  if ((executor = (executor_t *)malloc(sizeof(*executor))) == NULL) {
    return NULL;
  }
  *executor = (executor_t){
      .impl = ptr,
      .spawn = spawn,
      .run = run,
      .has_tasks = has_tasks,
      .free = free,
  };
  return executor;
}