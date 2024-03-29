// Type declaration for single work sharing descriptor
typedef struct {
    // complete the definition of single descriptor
    int key;
    pthread_mutex_t mutex;
    pthread_barrier_t barrier;
//    bool single;
} miniomp_single_t;

// Declaration of global variable for single work descriptor
extern miniomp_single_t miniomp_single;

// Functions implemented in this module
bool GOMP_single_start (void);
