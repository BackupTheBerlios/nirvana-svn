#include "pthread_once.h"

static pthread_mutex_t __pthread_once_mutex =  PTHREAD_MUTEX_INITIALIZER;

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
	/* Check first for speed */
	if (*once_control == PTHREAD_ONCE_INIT) {
		pthread_mutex_lock(&__pthread_once_mutex);
		if (*once_control == PTHREAD_ONCE_INIT) {
			init_routine();
			(*once_control)++;
		}
		pthread_mutex_unlock(&__pthread_once_mutex);
	}
	return(0);
}
