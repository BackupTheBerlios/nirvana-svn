#include "pthread.h"

typedef	 int				pthread_once_t;

#define PTHREAD_ONCE_INIT	0

int pthread_once(pthread_once_t *, void (*init_routine)(void));

