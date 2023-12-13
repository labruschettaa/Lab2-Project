#define _GNU_SOURCE   // permette di usare estensioni GNU
#include <stdio.h>    // permette di usare scanf printf etc ...
#include <stdlib.h>   // conversioni stringa exit() etc ...
#include <stdbool.h>  // gestisce tipo bool
#include <assert.h>   // permette di usare la funzione ass
#include <string.h>   // funzioni per stringhe
#include <errno.h>    // richiesto per usare errno
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <pthread.h>
#include <search.h>


void fun_termina(const char *messaggio);

int fun_thread_mutex_lock(pthread_mutex_t *mutex, int line, char *file);

int fun_thread_mutex_unlock(pthread_mutex_t *mutex, int line, char *file);

int fun_sem_init(sem_t *sem, int pshared, unsigned int value, int line, char *file);

int fun_sem_wait(sem_t *sem, int line, char *file);

int fun_sem_post(sem_t *sem, int line, char *file);

int fun_open(const char* name, int flag, int line, char *file);

int fun_read(int fd, void *buffer, size_t nbytes, int line, char *file);

int fun_thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg, int line, char *file);

int fun_thread_join(pthread_t thread, void **retval, int line, char *file);

int fun_pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr, int line, char *file);

int fun_pthread_cond_destroy(pthread_cond_t *cond, int line, char *file);

int fun_pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, int line, char *file);

int fun_pthread_cond_signal(pthread_cond_t *cond, int line, char *file);

int fun_thread_cond_broadcast(pthread_cond_t *cond, int line, char *file);

int fun_pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int line, char *file);

int fun_pthread_mutex_destroy(pthread_mutex_t *mutex, int line, char *file);