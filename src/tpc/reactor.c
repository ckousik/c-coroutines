#include "reactor.h"
#include <stdlib.h>

struct reactor_t {
  reactor_impl *impl;
  _Bool (*react)(reactor_impl *);
  void (*free)(reactor_impl *);
};

inline _Bool reactor_t_react(reactor_t *reactor) {
  return reactor->react(reactor->impl);
}

inline void reactor_t_free(reactor_t *reactor) {
  reactor->free(reactor->impl);
  free(reactor);
}

reactor_t *reactor_t_alloc(reactor_impl *impl, _Bool (*react)(reactor_impl *),
                           void (*free)(reactor_impl *)) {
  __auto_type reactor = (reactor_t *)malloc(sizeof(reactor_t));
  if (reactor == NULL) {
    return NULL;
  }
  *reactor = (reactor_t){
      .impl = impl,
      .react = react,
      .free = free,
  };
  return reactor;
}

inline reactor_impl* reactor_t_impl(reactor_t* reactor) {
  return reactor->impl;
}
