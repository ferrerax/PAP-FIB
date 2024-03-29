#include "heat.h"
#ifdef _OPENMP
#include <omp.h>
#endif

/*
 * Blocked Jacobi solver: one iteration step
 */
double relax_jacobi( double *u, double *utmp, unsigned sizex, unsigned sizey)
{
    double diff, sum=0.0;
    
    #pragma omp parallel private (diff) reduction(+:sum) 
    {
    int howmany=omp_get_num_threads();
    int block = omp_get_thread_num();
    int i_start = block * (sizex/howmany) + (block < (sizex%howmany) ? block : sizex % howmany);
    int i_end = i_start + (sizex/howmany) + (block < (sizex%howmany)) - 1; 
        for( int i=max(1,i_start); i<=min(sizex-2, i_end); i++ ) {
            for( int j=1; j<sizey-1; j++ ) {
                utmp[i*sizey + j] = 0.20 * (u[ i*sizey     + j     ]+  // center
                                            u[ i*sizey     + (j-1) ]+  // left
                                            u[ i*sizey     + (j+1) ]+  // right
                                            u[ (i-1)*sizey + j     ]+  // top
                                            u[ (i+1)*sizey + j     ]); // bottom

                diff = utmp[i*sizey + j] - u[i*sizey + j];
                sum += diff * diff;
            }
        }
    }
    return sum;
}
