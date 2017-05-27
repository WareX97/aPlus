
#include <stdint.h>
#include <pthread.h>

#include "pthread_internal.h"

PUBLIC int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
	errno = ENOSYS;
	return -1;
}