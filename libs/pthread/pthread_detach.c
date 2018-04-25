#include "pthread_internal.h"

int pthread_detach(pthread_t th) {
    if(th <= 0)
        return -1;

    struct p_context* cc = (struct p_context*) th;

    if(kill(cc->pid, SIGKILL) != 0)
        return -1;

    return 0;
}