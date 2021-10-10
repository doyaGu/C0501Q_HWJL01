/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "SDL_thread.h"
#include "SDL_timer.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"

/* Wrapper around POSIX 1003.1b semaphores */

struct SDL_semaphore
{
    sem_t sem;
};

/* Create a semaphore, initialized with value */
SDL_sem *
SDL_CreateSemaphore(Uint32 initial_value)
{
    SDL_sem *sem = (SDL_sem *) SDL_malloc(sizeof(SDL_sem));
    if (sem) {
        if (sem_init(&sem->sem, 0, initial_value) < 0) {
            SDL_SetError("sem_init() failed");
            SDL_free(sem);
            sem = NULL;
        }
    } else {
        SDL_OutOfMemory();
    }
    return sem;
}

void
SDL_DestroySemaphore(SDL_sem * sem)
{
    if (sem) {
        sem_destroy(&sem->sem);
        SDL_free(sem);
    }
}

int
SDL_SemTryWait(SDL_sem * sem)
{
    int retval;

    if (!sem) {
        SDL_SetError("Passed a NULL semaphore");
        return -1;
    }
    retval = SDL_MUTEX_TIMEDOUT;
    if (sem_trywait(&sem->sem) == 0) {
        retval = 0;
    }
    return retval;
}

int
SDL_SemWait(SDL_sem * sem)
{
    int retval;

    if (!sem) {
        SDL_SetError("Passed a NULL semaphore");
        return -1;
    }

    retval = sem_wait(&sem->sem);
    if (retval < 0) {
        SDL_SetError("sem_wait() failed");
    }
    return retval;
}

int
SDL_SemWaitTimeout(SDL_sem * sem, Uint32 timeout)
{
    int retval;

    if (!sem) {
        SDL_SetError("Passed a NULL semaphore");
        return -1;
    }

    /* Try the easy cases first */
    if (timeout == 0) {
        return SDL_SemTryWait(sem);
    }
    if (timeout == SDL_MUTEX_MAXWAIT) {
        return SDL_SemWait(sem);
    }

    retval = (pdTRUE == xSemaphoreTake(sem->sem.__sem_lock, timeout * portTICK_PERIOD_MS)) ? 0 : SDL_MUTEX_TIMEDOUT;
    return retval;
}

Uint32
SDL_SemValue(SDL_sem * sem)
{
    int ret = 0;
    if (sem) {
        sem_getvalue(&sem->sem, &ret);
        if (ret < 0) {
            ret = 0;
        }
    }
    return (Uint32) ret;
}

int
SDL_SemPost(SDL_sem * sem)
{
    int retval;

    if (!sem) {
        SDL_SetError("Passed a NULL semaphore");
        return -1;
    }

    retval = sem_post(&sem->sem);
    if (retval < 0) {
        SDL_SetError("sem_post() failed");
    }
    return retval;
}

/* vi: set ts=4 sw=4 expandtab: */
