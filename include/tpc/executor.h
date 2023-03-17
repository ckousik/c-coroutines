#ifndef __TPC_EXECUTOR_H_
#define __TPC_EXECUTOR_H_

#include "task.h"

struct executor_t;
struct executor_handle_t;

typedef struct executor_t executor_t;

typedef void executor_impl;

executor_t *executor_t_alloc(executor_impl *ptr,
                             int (*spawn)(executor_impl *, task_t *),
                             void (*run)(executor_impl *),
                             _Bool (*has_tasks)(executor_impl *),
                             void (*free)(executor_impl *));

// spawn a task on an executor
int executor_t_spawn(executor_t *, task_t *);
void executor_t_run(executor_t *);
_Bool executor_t_has_tasks(executor_t *);

void executor_t_free(executor_t *);

#endif
