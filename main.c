#include "tpc/executor.h"
#include "tpc/reactor.h"
#include "tpc/simple_thread_executor.h"
#include "uring_reactor/uring_reactor.h"

#include <stdio.h>
#include <stdlib.h>

// awaiting task based on Duff's device
typedef struct {
  unsigned int execution_state;
  executor_t *executor;
  task_t *prev;
  const char *message;
} hello_task_t;

task_state_t hello_task_t_poll(const task_impl *impl, waker_t *waker) {
  __auto_type task = (hello_task_t *)impl;
  switch (task->execution_state) {
  case 0:
    do {
      // initialized task, spawn the new future
      executor_t_spawn(task->executor, task->prev);
      task->execution_state = 1;
      return READY;
    case 1:
      if (task_t_state(task->prev) != COMPLETED) {
        return READY;
      }
      task->execution_state = 2;
    case 2:
      printf("%s\n", task->message);
      task->execution_state = 3;
      return COMPLETED;
    } while (task->execution_state);
  }
  return 0;
}

task_state_t hello_task_t_state(const task_impl *impl) {
  __auto_type task = (hello_task_t *)impl;
  switch (task->execution_state) {
  case 0:
  case 1:
  case 2:
    return READY;
  default:
  }
  return COMPLETED;
}

const void *hello_task_t_output(const task_impl *impl) { return NULL; }

void hello_task_t_free(task_impl *impl) {
  __auto_type task = (hello_task_t *)impl;
  // free owned task
  task_t_free(task->prev);
  free(impl);
}

task_t *hello_task_t_create(executor_t *executor, task_t *prev,
                            const char *message) {
  __auto_type hello = (hello_task_t *)malloc(sizeof(hello_task_t));
  if (hello == NULL) {
    return NULL;
  }
  *hello = (hello_task_t){
      .execution_state = 0,
      .executor = executor,
      .prev = prev,
      .message = message,
  };
  return alloc_raw_task(hello, hello_task_t_poll, hello_task_t_state,
                        hello_task_t_output, hello_task_t_free);
}

int main() {
  executor_t *executor = simple_thread_executor_t_create();
  reactor_t *reactor = uring_reactor_t_create((uring_reactor_config_t){
      .entries = 1024,
  });
  task_t *timer_task_0 =
      uring_timer_task_t_create(reactor, (struct __kernel_timespec){
                                             .tv_nsec = 0,
                                             .tv_sec = 1,
                                         });
  task_t *timer_task_1 =
      uring_timer_task_t_create(reactor, (struct __kernel_timespec){
                                             .tv_nsec = 0,
                                             .tv_sec = 2,
                                         });

  task_t *hello = hello_task_t_create(executor, timer_task_0, "Hello, ");
  task_t *world = hello_task_t_create(executor, timer_task_1, "World!\n");

  executor_t_spawn(executor, hello);
  executor_t_spawn(executor, world);

  printf("Executor starting\n");
  while (executor_t_has_tasks(executor)) {
    executor_t_run(executor);
    reactor_t_react(reactor);
  }

  task_t_free(hello);
  task_t_free(world);
  printf("Executor complete\n");
  reactor_t_free(reactor);
  executor_t_free(executor);
  return 0;
}
