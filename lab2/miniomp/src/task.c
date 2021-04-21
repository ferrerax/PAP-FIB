#include "libminiomp.h"

miniomp_taskqueue_t * miniomp_taskqueue;

// Initializes the task queue
miniomp_taskqueue_t *TQinit(int max_elements) {
    return NULL;
}

// Checks if the task queue is empty
bool TQis_empty(miniomp_taskqueue_t *task_queue) {
    return true;
}

// Checks if the task queue is full
bool TQis_full(miniomp_taskqueue_t *task_queue) {
    return false;
}

// Enqueues the task descriptor at the tail of the task queue
bool TQenqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor) {
    return true;
}

// Dequeue the task descriptor at the head of the task queue
bool TQdequeue(miniomp_taskqueue_t *task_queue) { 
    return true;
}

// Returns the task descriptor at the head of the task queue
miniomp_task_t *TQfirst(miniomp_taskqueue_t *task_queue) {
    return NULL;
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
    printf("TBI: a task has been encountered, I am executing it immediately\n");

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

    // Function invocation should be replaced with the appropriate task instatiation
    fn (arg);
}
