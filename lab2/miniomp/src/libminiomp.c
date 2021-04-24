#include "libminiomp.h"

// Library constructor and desctructor
void init_miniomp(void) __attribute__((constructor));
void fini_miniomp(void) __attribute__((destructor));

void
init_miniomp(void) {
  printf ("mini-omp is being initialized\n");
  // Parse OMP_NUM_THREADS environment variable to initialize nthreads_var internal control variable
  parse_env();
  // Initialize pthread and parallel data structures 

  // Initialize Pthread thread-specific data, now just used to store the OpenMP thread identifier
  pthread_key_create(&miniomp_specifickey, NULL);
  pthread_setspecific(miniomp_specifickey, (void *) 0); // implicit initial pthread with id=0
  // Initialize OpenMP default locks and default barrier
  pthread_mutex_init(&miniomp_default_lock, NULL);
  pthread_mutex_init(&miniomp_named_lock, NULL);
  pthread_barrier_init(&miniomp_barrier, NULL, omp_get_num_threads());
  // Initialize OpenMP workdescriptors for single 
  pthread_mutex_init(&miniomp_single.mutex, NULL);
  pthread_barrier_init(&miniomp_single.barrier, NULL, omp_get_num_threads());
  // Initialize OpenMP task queue for task and taskloop
  miniomp_taskqueue = TQinit(10000); // li poso un numero ben grandot.
}

void
fini_miniomp(void) {
  // delete Pthread thread-specific data
  pthread_key_delete(miniomp_specifickey);

  // free other data structures allocated during library initialization
  printf ("mini-omp is finalized\n");
}
