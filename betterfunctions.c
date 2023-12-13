#include "betterfunctions.h"
#define Buflen 100

void fun_perror(int en, char *msg) {
  char buf[Buflen];
  char *errmsg = strerror_r(en, buf, Buflen);
  if(msg!=NULL)
    fprintf(stderr,"%s: %s\n",msg, errmsg);
  else
    fprintf(stderr,"%s\n",errmsg);
}

void fun_termina(const char *messaggio){
  if(errno!=0) perror(messaggio);
	else fprintf(stderr,"%s\n", messaggio);
  exit(1);
}

int fun_sem_wait(sem_t *sem, int line, char *file){
  int e = sem_wait(sem);
  if (e!=0){
    perror("Errore sem_wait"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_thread_mutex_lock(pthread_mutex_t *mutex, int line, char *file) {
  int e = pthread_mutex_lock(mutex);
  if (e!=0) {
    fun_perror(e, "Errore pthread_mutex_lock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_thread_mutex_unlock(pthread_mutex_t *mutex, int line, char *file) {
  int e = pthread_mutex_unlock(mutex);
  if (e!=0) {
    fun_perror(e, "Errore pthread_mutex_unlock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_sem_post(sem_t *sem, int line, char *file) {
  int e = sem_post(sem);
  if(e !=0) {
    perror("Errore sem_post"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_open(const char* name, int flag, int line, char *file){
  int e = open(name,flag);
  if(e<0){
    fun_perror(e, "Errore open");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_read(int fd, void *buffer, size_t nbytes, int line, char *file){
  int e = read(fd,buffer,nbytes);
  if(e==-1){
    fun_perror(e, "Errore open");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_sem_init(sem_t *sem, int pshared, unsigned int value, int line, char *file){
  int e = sem_init(sem,pshared,value);
  if(e !=0) {
    perror("Errore sem_init"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    exit(1);
  }
  return e;
}

int fun_thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg, int line, char *file) {
  int e = pthread_create(thread, attr, start_routine, arg);
  if (e!=0) {
    fun_perror(e, "Errore pthread_create");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;                       
}
                          
int fun_thread_join(pthread_t thread, void **retval, int line, char *file) {

  int e = pthread_join(thread, retval);
  if (e!=0) {
    fun_perror(e, "Errore pthread_join");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr, int line, char *file){
  int e = pthread_cond_init(cond,attr);
  if(e!=0){
    fun_perror(e,"Errore pthread_cond_init");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_pthread_cond_destroy(pthread_cond_t *cond, int line, char *file) {
  int e = pthread_cond_destroy(cond);
  if (e!=0) {
    fun_perror(e, "Errore pthread_cond_destroy");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, int line, char *file) {
  int e = pthread_cond_wait(cond,mutex);
  if (e!=0) {
    fun_perror(e, "Errore pthread_cond_wait");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_pthread_cond_signal(pthread_cond_t *cond, int line, char *file) {
  int e = pthread_cond_signal(cond);
  if (e!=0) {
    fun_perror(e, "Errore pthread_cond_signal");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_thread_cond_broadcast(pthread_cond_t *cond, int line, char *file) {
  int e = pthread_cond_broadcast(cond);
  if (e!=0) {
    fun_perror(e, "Errore pthread_cond_broadcast");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}

int fun_pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int line, char *file) {
  int e = pthread_mutex_init(mutex, attr);
  if (e!=0) {
    fun_perror(e, "Errore pthread_mutex_init");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }  
  return e;
}

int fun_pthread_mutex_destroy(pthread_mutex_t *mutex, int line, char *file) {
  int e = pthread_mutex_destroy(mutex);
  if (e!=0) {
    fun_perror(e, "Errore pthread_mutex_destroy");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    pthread_exit(NULL);
  }
  return e;
}