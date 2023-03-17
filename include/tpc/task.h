#ifndef __TPC_TASK_H_
#define __TPC_TASK_H_

#include "waker.h"

typedef enum {
  READY,
  PENDING,
  COMPLETED,
} task_state_t;

struct task_t;
typedef struct task_t task_t;

typedef void task_impl;

task_t *alloc_raw_task(task_impl *ptr,
                       task_state_t (*poll)(const task_impl *, waker_t *),
                       task_state_t (*state)(const task_impl *),
                       const void *(*output)(const task_impl *),
                       void (*free)(task_impl *));

task_state_t task_t_poll(task_t *, waker_t *);
task_state_t task_t_state(task_t *);
const void *task_t_output(task_t *);
void task_t_free(task_t *);

#endif
