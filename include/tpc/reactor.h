#ifndef __TPC_REACTOR_H_
#define __TPC_REACTOR_H_

typedef struct reactor_t reactor_t;
typedef void reactor_impl;

// allocate a new Reactor
reactor_t *reactor_t_alloc(reactor_impl *impl, _Bool (*react)(reactor_impl *),
                           void (*free)(reactor_impl *));

// React to any events and wake up any tasks.
_Bool reactor_t_react(reactor_t *);

// Free the reactor instance
void reactor_t_free(reactor_t *);

reactor_impl* reactor_t_impl(reactor_t*);

#endif
