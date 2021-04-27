#include "libminiomp.h"

miniomp_taskqueue_t * miniomp_taskqueue;

// Initializes the task queue
miniomp_taskqueue_t *TQinit(int max_elements) {
    miniomp_taskqueue_t * taskqueue = malloc(sizeof(miniomp_taskqueue_t));
    taskqueue->max_elements = max_elements;
    taskqueue->count = 0;
//    taskqueue->head = NULL;
    taskqueue->tail = 0;
    taskqueue->first = 0;
    taskqueue->queue=malloc(max_elements*sizeof(miniomp_task_t *));
    pthread_mutex_init(&taskqueue->lock_queue, NULL);
    pthread_barrier_init(&taskqueue->barrier, NULL, omp_get_num_threads());
    taskqueue->taskgroup_tasks=0;
    taskqueue->taskgroup=false;
    return taskqueue;
}


// Checks if the task queue is empty
bool TQis_empty(miniomp_taskqueue_t *task_queue) {
        bool a;
        a = (task_queue->count == 0);
  	return a;
}

// Checks if the task queue is full
bool TQis_full(miniomp_taskqueue_t *task_queue) {
        bool a;
        a = (task_queue->count == task_queue->max_elements);
        return a;
}

// Enqueues the task descriptor at the tail of the task queue
bool TQenqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor) {
        bool a;
        pthread_mutex_lock(&task_queue->lock_queue);
        if (TQis_full(task_queue))
            a = false;
        else 
        {
            if (task_queue->tail >= task_queue->max_elements) task_queue->tail = 0;  //la volta al buffer circular.
            task_queue->count++;
            task_queue->queue[task_queue->tail] = task_descriptor;
            //if ( == NULL) task_queue->head = task_descriptor;
            task_queue->tail++;
            //printf("thread %d ha posat la tasca %p\n", omp_get_thread_num(),task_descriptor);
            a = true;
        }
       pthread_mutex_unlock(&task_queue->lock_queue);
       return a;
}

// Dequeue the task descriptor at the head of the task queue
bool TQdequeue(miniomp_taskqueue_t *task_queue) { 
        bool a;
        if (TQis_empty(task_queue))
            a = false;
        else
        {
            //printf("Punter %p, thread %d\n", task_queue->head, omp_get_thread_num());
            free(task_queue->queue[task_queue->first]); //borro
            task_queue->count -= 1;
            task_queue->first++;
            //task_queue->head = task_queue->queue[task_queue->first];
            if (task_queue->first == task_queue->max_elements) task_queue->first = 0;
        }
    return a;
}

// Returns the task descriptor at the head of the task queue. TAMBE FA DEQUEUE!!!
bool TQfirst(miniomp_taskqueue_t *task_queue, miniomp_task_t * task_return) {
       bool a;
       miniomp_task_t task;
       pthread_mutex_lock(&task_queue->lock_queue);
       if(TQis_empty(task_queue)){
           a = false; // si retorna fals es que no hi ha tasques a pillar
       } else {
           miniomp_task_t * p_task = task_queue->queue[task_queue->first];
           //printf("obtinc la tasca %p de la posicio %d del vector",p_task,task_queue->first);
           task = *p_task;
           TQdequeue(task_queue);
           a = true;
       }
       pthread_mutex_unlock(&task_queue->lock_queue);
       *task_return = task;  // la historia esta buida retorna NULL
       return a;
}

#define GOMP_TASK_FLAG_UNTIED           (1 << 0)
#define GOMP_TASK_FLAG_FINAL            (1 << 1)
#define GOMP_TASK_FLAG_MERGEABLE        (1 << 2)
#define GOMP_TASK_FLAG_DEPEND           (1 << 3)
#define GOMP_TASK_FLAG_PRIORITY         (1 << 4)
#define GOMP_TASK_FLAG_UP               (1 << 8)
#define GOMP_TASK_FLAG_GRAINSIZE        (1 << 9)
#define GOMP_TASK_FLAG_IF               (1 << 10)
#define GOMP_TASK_FLAG_NOGROUP          (1 << 11)
#define GOMP_TASK_FLAG_REDUCTION        (1 << 12)

// Called when encountering an explicit task directive. Arguments are:
//      1. void (*fn) (void *): the generated outlined function for the task body
//      2. void *data: the parameters for the outlined function
//      3. void (*cpyfn) (void *, void *): copy function to replace the default memcpy() from 
//                                         function data to each task's data
//      4. long arg_size: specify the size of data
//      5. long arg_align: alignment of the data
//      6. bool if_clause: the value of if_clause. true --> 1, false -->0; default is set to 1 by compiler
//      7. unsigned flags: see list of the above

void
GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
           long arg_size, long arg_align, bool if_clause, unsigned flags,
           void **depend, int priority)
{
    //printf("TBI: thread %d entra al task\n", omp_get_thread_num());

    // This part of the code appropriately copies data to be passed to task function,
    // either using a compiler cpyfn function or just memcopy otherwise; no need to
    // fully understand it for the purposes of this assignment
    char *arg;
    if (__builtin_expect (cpyfn != NULL, 0)) {
	  char *buf =  malloc(sizeof(char) * (arg_size + arg_align - 1));
          arg       = (char *) (((uintptr_t) buf + arg_align - 1)
                               & ~(uintptr_t) (arg_align - 1));
          cpyfn (arg, data);
    } else {
          arg       =  malloc(sizeof(char) * (arg_size + arg_align - 1));
          memcpy (arg, data, arg_size);
    }
    miniomp_task_t * task = malloc(sizeof(miniomp_task_t));
    task->fn = fn; //carrego la funcio
    task->data = data; //obtinc les dades 
    if (miniomp_taskqueue->taskgroup) {
        task->taskgroup=true;
        miniomp_taskqueue->taskgroup_tasks++;
    }
    else 
    {
        task->taskgroup=false;
    } 
    //printf("creo tasca %p amb fn %p, data %p\n",task,task->fn,data);
    if (! TQenqueue(miniomp_taskqueue,task)) {
        perror("No s'ha pogut inserir la nova tasca a la cua de tasques");
        exit(1);
    }
    
    //printf("TBI: thread %d surt del task\n", omp_get_thread_num());
}

//Funcio per a l'execucio de la cua de tasques

void buida_cua_tasques(){
    miniomp_task_t task;
    bool hi_ha_tasks = TQfirst(miniomp_taskqueue, &task);
    while(hi_ha_tasks)  //Seguim tenint tasques a pillar
    {
       //printf("Executo la tasca %p\n", task);
       //printf("Amb la funcio %p\n", task->fn);
       if (task.taskgroup)
           miniomp_taskqueue->taskgroup_tasks--;
       task.fn(task.data);
       hi_ha_tasks = TQfirst(miniomp_taskqueue, &task);
    }
}
