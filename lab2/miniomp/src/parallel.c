#include "libminiomp.h"

// This file implements the PARALLEL construct as part of your 
// miniomp runtime library. parallel.h contains definitions of 
// data types used here

// Declaration of array for storing pthread identifier from 
// pthread_create function
pthread_t *miniomp_threads;

// Global variable for parallel descriptor
miniomp_parallel_t *miniomp_parallel;

// Declaration of per-thread specific key
pthread_key_t miniomp_specifickey;

// Variable per a la gestio dels threads
int executant;

// This is the prototype for the Pthreads starting function

void *
worker(void *args) {
	miniomp_parallel_t * thread = args;
  // insert all necessary code here for:
  //   1) save thread-specific data
        pthread_setspecific(miniomp_specifickey, (void *)(intptr_t) thread->id);	
  //   2) invoke the per-threads instance of function encapsulating the parallel region
	thread->fn(thread->fn_data);
  //   3) Executar tasques 
        buida_cua_tasques(); //Poso el thread a executar tasques
        //Atencio que aqui m'he marcat un triple, pot estar mal -->https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins
        __atomic_sub_fetch(&executant,1,__ATOMIC_RELAXED); // shan acabat les tasquest i ho indico
        printf("Soc el thread %d i executant ara val %d perque he acabat la execucio\n",omp_get_thread_num(), executant);
        //em poso en espera activa pq hi ha threads que podrien crear tasques.
        while(executant)  //Aixo es molt poc eficient --> molts fallos en cache!!!! cal millorar.
            buida_cua_tasques();
  //   4) exit the function
  printf("El thread %d ha acabat definitivament", omp_get_thread_num());
  pthread_exit(NULL);
}

void
GOMP_parallel (void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags) {
  int anticThreads = omp_get_num_threads();
  executant=0; //No estic contant el pare.
  if(!num_threads){
	num_threads = omp_get_num_threads();
  } else {    // He de tornar a iniciar les barreres pq canvia el num de threads
  	omp_set_num_threads(num_threads);
	pthread_barrier_init(&miniomp_barrier, NULL, num_threads);
  	pthread_barrier_init(&miniomp_single.barrier, NULL, num_threads);
        pthread_barrier_init(&miniomp_taskqueue->barrier, NULL, num_threads);
  }
  //printf("Starting a parallel region using %d threads\n", num_threads);
  miniomp_threads=malloc(sizeof(pthread_t)*num_threads);
  miniomp_parallel=malloc(sizeof(miniomp_parallel_t)*num_threads);
  // Per si de cas cal guardar aquesta info del thread pare:
  miniomp_parallel[0].fn = fn;
  miniomp_parallel[0].fn_data = data;
  miniomp_parallel[0].id = 0;
  //Creacio dels fills
  for (int i=1; i<num_threads; i++){
	miniomp_parallel[i].fn = *fn;
	miniomp_parallel[i].fn_data = data;
	miniomp_parallel[i].id = i;
        executant++;
	pthread_create(&miniomp_threads[i],NULL,&worker,&miniomp_parallel[i]);
  }     
  fn (data);
  buida_cua_tasques();  // el main thread tambe ha de buidar la cua de tasques!!
  printf("Soc el thread 0 he acabat. No executare mes tasques.\n");
  for (int i=1; i<num_threads; i++){
	pthread_join(miniomp_threads[i],NULL);  //Barrera implicita.
  }
  free(miniomp_threads);
  free(miniomp_parallel);
  omp_set_num_threads(anticThreads);   //Retorno el valor que tenien els threads.
}
