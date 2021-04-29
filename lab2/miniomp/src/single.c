#include "libminiomp.h"

// Declaratiuon of global variable for single work descriptor
miniomp_single_t miniomp_single;

// This routine is called when first encountering a SINGLE construct. 
// Returns true if this is the thread that should execute the clause. 

bool
GOMP_single_start (void) {
  //printf("TBI: Entering into single, don't know if anyone else arrived before, I proceed\n");
  bool ret = false;
  pthread_mutex_lock(&miniomp_single.mutex);
  if (!miniomp_single.key){   //Puc executar la regió critica
	ret = true;
        pthread_barrier_init(&miniomp_single.barrier,NULL,omp_get_num_threads());
	miniomp_single.key++;
  }	  
  pthread_mutex_unlock(&miniomp_single.mutex);
  pthread_barrier_wait(&miniomp_single.barrier); //Tots els threads han d'haver arribat abans de treure la key.
  miniomp_single.key = 0;
  return (ret);
}
