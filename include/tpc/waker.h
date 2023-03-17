#ifndef __TPC_WAKER_H_
#define __TPC_WAKER_H_

struct waker_t;
typedef struct waker_t waker_t;
typedef void waker_impl;

waker_t *waker_t_alloc(
    // raw pointer to the underlying waker
    waker_impl *,
    // waker function
    void (*wake)(const waker_impl *),
    // free the underlying waker
    void (*free)(waker_impl *));

// wake
void waker_t_wake(waker_t *);

// free
void waker_t_free(waker_t *);

#endif
