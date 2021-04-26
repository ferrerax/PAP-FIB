#include "libminiomp.h"

// Default lock for unnamed critical sections
pthread_mutex_t miniomp_default_lock;
pthread_mutex_t miniomp_named_lock;

void 
GOMP_critical_start (void) {
  //printf("TBI: Entering an unnamed critical, don't know if anyone else is alrady in. I proceed\n");
  pthread_mutex_lock(&miniomp_default_lock); 
}

void 
GOMP_critical_end (void) {
  //printf("TBI: Exiting an unnamed critical section. I can not inform anyone else, bye!\n");
  pthread_mutex_unlock(&miniomp_default_lock);
}

void 
GOMP_critical_name_start (void **pptr) {
  pthread_mutex_t * plock = *pptr;
  pthread_mutex_lock(&miniomp_named_lock);
  if (plock == NULL){
	plock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(plock,NULL);
	*pptr = plock;
  }
  pthread_mutex_unlock(&miniomp_named_lock);
  pthread_mutex_lock(plock);
  //printf("TBI: Entering a named critical %p (%p), don't know if anyone else is alrady in. I proceed\n", pptr, plock);
}

void 
GOMP_critical_name_end (void **pptr) {
  pthread_mutex_t * plock = *pptr;
  pthread_mutex_unlock(plock);
  // if plock is still NULL something went wrong
  //printf("TBI: Exiting a named critical %p (%p), I can not inform anyone else, bye!\n", pptr, plock);
}

// Default barrier within a parallel region
pthread_barrier_t miniomp_barrier;

void 
GOMP_barrier() {
  //printf("TBI: Entering in barrier, but do not know how to wait for the rest. I proceed\n");
  pthread_barrier_wait(&miniomp_barrier);
  buida_cua_tasques();
}
