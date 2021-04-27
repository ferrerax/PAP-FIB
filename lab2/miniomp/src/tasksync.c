#include "libminiomp.h"
#include <stdlib.h>

// Called when encountering taskwait and taskgroup constructs

void
GOMP_taskwait (void)
{
//    printf("TBI: Entered in taskwait, there should be no pending tasks, so I proceed\n");

    buida_cua_tasques();

//    __atomic_sub_fetch(&executant,1,__ATOMIC_RELAXED); // shan acabat les tasquest i ho indico
//    printf("Soc el thread %d i executant ara val %d perque estic en un taskwait", omp_get_thread_num(), executant);
//    //em poso en espera activa pq hi ha threads que podrien crear tasques.
//    while(executant)  //Aixo es molt poc eficient --> molts fallos en cache!!!! cal millorar.
//        buida_cua_tasques();
//    pthread_barrier_wait(&miniomp_taskqueue->barrier); //Esperem fins que tots han executat les tasques que esperaven.
//    executant = omp_get_num_threads();
}

void
GOMP_taskgroup_start (void)
{
    //printf("TBI: Starting a taskgroup region, at the end of which I should wait for tasks created here\n");
    //Considero tenir les tasques del taskgroup en una altre cua. --> No perque estaria donant prioritat a aquestes. Problema que podem tenir--> que un altre thread crei les tasques que no siguin del taskgroup, es consideraran del taskgroup.
    miniomp_taskqueue->taskgroup=true;
    
}

void
GOMP_taskgroup_end (void)
{
    while(miniomp_taskqueue->taskgroup_tasks){
        buida_cua_tasques();
    }
    __atomic_sub_fetch(&executant,1,__ATOMIC_RELAXED); // shan acabat les tasquest i ho indico
    printf("Soc el thread %d i executant ara val %d perque estic en un end taskgroup", omp_get_thread_num(), executant);
    while(executant || miniomp_taskqueue->taskgroup_tasks)  
        buida_cua_tasques();
    pthread_barrier_wait(&miniomp_taskqueue->barrier); 
    executant = omp_get_num_threads();
    miniomp_taskqueue->taskgroup=false;
}
