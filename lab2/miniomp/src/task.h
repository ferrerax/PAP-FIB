    /* This structure describes a "task" to be run by a thread.  */
    typedef struct {
    void (*fn)(void *);
    void (*data);
    bool taskgroup;
} miniomp_task_t;

typedef struct {
    int max_elements;
    int count;
//    miniomp_task_t * head; // un punter al cap
    int tail;
    int first;    //un punter a l'index del vector que conte el primer element.
    pthread_mutex_t lock_queue; //per a que les consultes siguin atomiques.
    miniomp_task_t **queue;
    pthread_barrier_t barrier;
    int taskgroup_tasks;
    bool taskgroup;
    int taskwaitcount;
} miniomp_taskqueue_t;

extern miniomp_taskqueue_t * miniomp_taskqueue;
#define MAXELEMENTS_TQ 128

// funtions to implement basic management operations on taskqueue
miniomp_taskqueue_t *TQinit(int max_elements);
bool TQis_empty(miniomp_taskqueue_t *task_queue);
bool TQis_full(miniomp_taskqueue_t *task_queue) ;
bool TQenqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor); 
bool TQdequeue(miniomp_taskqueue_t *task_queue);
bool TQfirst(miniomp_taskqueue_t *task_queue,miniomp_task_t * task_return); 

// Functions implemented in task* modules
void GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
           long arg_size, long arg_align, bool if_clause, unsigned flags,
           void **depend, int priority);
void GOMP_taskloop (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
               long arg_size, long arg_align, unsigned flags,
               unsigned long num_tasks, int priority,
               long start, long end, long step);
void GOMP_taskwait (void);
void GOMP_taskgroup_start (void);
void GOMP_taskgroup_end (void);
void GOMP_taskgroup_reduction_register (uintptr_t *data);
void GOMP_taskgroup_reduction_unregister (uintptr_t *data);
void GOMP_task_reduction_remap (size_t cnt, size_t cntorig, void **ptrs);



void buida_cua_tasques();
