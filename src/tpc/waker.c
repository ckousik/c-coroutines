#include "waker.h"
#include <stdlib.h>

struct waker_t {
  waker_impl *ptr;
  void (*wake)(const waker_impl *);
  void (*free)(waker_impl *);
};

waker_t *waker_t_alloc(waker_impl *ptr, void (*wake)(const waker_impl *),
                       void (*free)(waker_impl *)) {
  waker_t *waker = NULL;
  if ((waker = (waker_t *)malloc(sizeof(waker_t))) == NULL) {
    return NULL;
  }
  *waker = (waker_t){
      .ptr = ptr,
      .wake = wake,
      .free = free,
  };
  return waker;
}

void waker_t_wake(waker_t *waker) { waker->wake(waker->ptr); }

void waker_t_free(waker_t *waker) { waker->free(waker->ptr); }
