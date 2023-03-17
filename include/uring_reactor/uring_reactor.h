#ifndef __URING_REACTOR_H__
#define __URING_REACTOR_H__

#include "reactor.h"
#include "task.h"
#include "liburing.h"

typedef struct {
  unsigned int entries;
} uring_reactor_config_t;

reactor_t *uring_reactor_t_create(uring_reactor_config_t);

task_t *uring_timer_task_t_create(reactor_t *,
                                  struct __kernel_timespec ts);

#endif
