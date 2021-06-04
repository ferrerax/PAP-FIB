/*
 * Iterative solver for heat distribution
 */

#include <stdio.h>
#include <stdlib.h>
#include "heat.h"
#include <mpi.h>

void usage( char *s )
{
    fprintf(stderr, "Usage: %s <input file> [result file]\n\n", s);
}

int main( int argc, char *argv[] )
{
    int columns, rows, proc_rows;
    int iter, maxiter;
    double residual=0.0;

    int myid, numprocs, len;
    MPI_Status status;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(hostname, &len);

    if (myid == 0) {
        printf("I am the master running on %s, distributing work to %d additional workers ...\n", hostname, numprocs-1);

        // Input and output files
        FILE *infile, *resfile;
        char *resfilename;

        // algorithmic parameters
        algoparam_t param;

        double runtime, flop;

        // check arguments
        if( argc < 2 ) {
            usage( argv[0] );
            return 1;
        }

        // check input file
        if( !(infile=fopen(argv[1], "r")) ) {
            fprintf(stderr, "\nError: Cannot open \"%s\" for reading.\n\n", argv[1]);
            usage(argv[0]);
            return 1;
        }

        // check result file
        resfilename= (argc>=3) ? argv[2]:"heat.ppm";

        if( !(resfile=fopen(resfilename, "w")) ) {
            fprintf(stderr, "\nError: Cannot open \"%s\" for writing.\n\n", resfilename);
            usage(argv[0]);
            return 1;
        }

        // check input
        if( !read_input(infile, &param) ) {
            fprintf(stderr, "\nError: Error parsing input file.\n\n");
            usage(argv[0]);
            return 1;
        }
        print_params(&param);

        if( !initialize(&param) ) {
            fprintf(stderr, "Error in Solver initialization.\n\n");
            usage(argv[0]);
            return 1;
        }

        maxiter = param.maxiter;
        // full size (param.resolution are only the inner points)
        columns = param.resolution + 2;
        rows = columns;

        // starting time
        runtime = wtime();
        
        //Calculs pel pare
        proc_rows = param.resolution/numprocs + ((param.resolution%numprocs) > 0); //per a repartir les iteracions

        // send to workers the necessary information to perform computation
        for (int i=0; i<numprocs; i++) {
            if (i>0) {
                proc_rows = param.resolution/numprocs + ((param.resolution%numprocs) > i); //per a repartir les iteracions... correcte?
                MPI_Send(&maxiter, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&columns, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&proc_rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&param.u[i * proc_rows * columns], (proc_rows+2)*columns, MPI_DOUBLE, i, 0, MPI_COMM_WORLD); //Situarem a la fila que toca pel tema del marge?
                MPI_Send(&param.uhelp[i * proc_rows * columns], (proc_rows+2)*columns, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            }
        }

        iter = 0;
        
        MPI_Request rep_fila_baixa,envia_fila_baixa;
        while(1) {
            if (iter > 0)
             {
                //Esperem a que s'hagi enviat la fila per no matxacar-la
//                MPI_Wait(&envia_fila_baixa, MPI_STATUS_IGNORE);
             }
            //printf("Soc el proces %d i comenco la it %d \n",myid,iter);
            //Demano el valor que necessitare per a la proxima iteracio
            printf("hola it=%d\n", iter);            
            double residual_tmp = relax_jacobi(param.u, param.uhelp, proc_rows+2, columns);

            if (iter < maxiter - 1)    
                MPI_Irecv(&param.u[(proc_rows+1)*columns],columns,MPI_DOUBLE,1,1,MPI_COMM_WORLD,&rep_fila_baixa);
            iter++;
            printf("adeu it=%d\n", iter);            

            //Si no hem arribat al limit de les iteracions hem de transmetre missatge
            MPI_Isend(&param.u[proc_rows*columns],columns,MPI_DOUBLE,1,1,MPI_COMM_WORLD,&envia_fila_baixa);
           
            //I esperem a rebre la fila per computar la seguent iteracio si no l-hem rebuda ja
            printf("Proces %d bloquejat a la it %d\n", myid, iter);
            MPI_Wait(&rep_fila_baixa, MPI_STATUS_IGNORE);
            printf("Proces %d desbloquejat a la it %d i residual %f\n", myid, iter, residual);
            
            // Copy uhelp into u
            double * tmp = param.u; param.u = param.uhelp; param.uhelp = tmp;
            
            // solution good enough ?
            MPI_Allreduce(&residual_tmp, &residual, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            
            if (residual < 0.00005){
                printf("ACAAAAAAABOOO proces %d amb iter=%d\n",myid,iter);
                break;
            }
            printf("maxiter=%d\n", maxiter);            

            // max. iteration reached ? (no limit with maxiter=0)
            if (maxiter>0 && iter>=maxiter){
                printf("ACAAAAAAABOOO amb iter=%d\n",iter);
                break;
            }
//            printf("I aqui estic amb iter = %d i maxiter=%d\n", iter,maxiter);
        }

        //HEM DE REBRE EL QUE HAN COMPUTAT ELS PROCESSOS FILLS!!
        
        for (int i=1; i<numprocs; i++) {
            proc_rows = param.resolution/numprocs + ((param.resolution%numprocs) > i); //per a repartir les iteracions... correcte?
            MPI_Recv(&param.u[(i * proc_rows * columns) + columns], proc_rows*columns, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Canvio el numero pq no es faci uin lio amb la comm
            //MPI_Recv(&param.uhelp[(i * proc_rows * columns) + columns], proc_rows*columns, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // stopping time
        runtime = wtime() - runtime;

        // Flop count after iter iterations
        flop = iter * 11.0 * param.resolution * param.resolution;

        fprintf(stdout, "Time: %04.3f \n", runtime);
        fprintf(stdout, "Flops and Flops per second: (%3.3f GFlop => %6.2f MFlop/s)\n",
                flop/1000000000.0,
                flop/runtime/1000000);
        fprintf(stdout, "Convergence to residual=%f: %d iterations\n", residual, iter);

        // for plot...
        if (param.resolution < 1024) {
            coarsen( param.u, rows, columns, param.uvis, param.visres+2, param.visres+2 );
            write_image( resfile, param.uvis, param.visres+2, param.visres+2 );
        }

        finalize( &param );

        MPI_Finalize();
        return 0;
    } else {
        printf("I am worker %d on %s and ready to receive work from master ...\n", myid, hostname);

        // receive information from master to perform computation locally
        MPI_Recv(&maxiter, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&columns, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&proc_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        rows = proc_rows+2;

        // allocate memory for worker
        double * u     = calloc( sizeof(double),rows*columns );
        double * uhelp = calloc( sizeof(double),rows*columns );

        if( !u || !uhelp ) {
            fprintf(stderr, "Error: Cannot allocate memory\n");
            return 0;
        }

        // fill initial values for matrix with values received from master
        MPI_Recv(&u[0], rows*columns, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&uhelp[0], rows*columns, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

        iter = 0;
        MPI_Request rep_fila_baixa,envia_fila_baixa,rep_fila_alta,envia_fila_alta;
        while(1) {
            //printf("Soc el proces %d i comenco la it %d \n",myid,iter);
            //Espero a haver enviat les files abans de calcular
            if (iter > 0)
             {
//                MPI_Wait(&envia_fila_alta, MPI_STATUS_IGNORE);
//                if (myid < numprocs -1 )
//                    MPI_Wait(&envia_fila_baixa, MPI_STATUS_IGNORE);
             }
            //Demano les files de dalt i de baix per a la proxima iteracio

            double residual_tmp = relax_jacobi(u, uhelp, rows, columns);

            if (iter < maxiter - 1){
                MPI_Irecv(&u[0],columns,MPI_DOUBLE,myid-1,1,MPI_COMM_WORLD,&rep_fila_alta);
                if (myid < numprocs - 1){
                    MPI_Irecv(&u[(rows-1)*columns],columns,MPI_DOUBLE,myid+1,1,MPI_COMM_WORLD,&rep_fila_baixa);
                }
            }
            iter++;

            //Si no hem arribat al limit de les iteracions hem de transmetre missatge
            MPI_Isend(&u[columns],columns,MPI_DOUBLE,myid-1,1,MPI_COMM_WORLD,&envia_fila_alta);
            if (myid<numprocs-1)
                MPI_Isend(&u[(rows-2)*columns],columns,MPI_DOUBLE,myid+1,1,MPI_COMM_WORLD,&envia_fila_baixa);
            //I esperem a rebre la fila per computar la seguent iteracio si no l-hem rebuda ja
//            printf("Proces %d bloquejat a la it %d\n", myid, iter);
            MPI_Wait(&rep_fila_alta,MPI_STATUS_IGNORE);
            if (myid<numprocs-1)
            {
                MPI_Wait(&rep_fila_baixa, MPI_STATUS_IGNORE);
            } 
            printf("Proces %d desbloquejat a la it %d i residual %f\n", myid, iter, residual);
            
            
            //reduccio per a obtenir el valor correcte de  residual
            MPI_Allreduce(&residual_tmp, &residual, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            
            if (residual < 0.00005){
                printf("ACAAAAAAABOOO proces %d amb iter=%d",myid,iter);
                break;
            }

            // max. iteration reached ? (no limit with maxiter=0)
            if (maxiter>0 && iter>=maxiter) break;

        }

        fprintf(stdout, "Process %d finished computing after %d iterations with residual value = %f\n", myid, iter, residual);

        MPI_Send(&u[columns], proc_rows*columns, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD); //Canvio el numero pq no es faci uin lio amb la comunicacio
//        MPI_Send(&uhelp[columns], proc_rows*columns, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);

        MPI_Finalize();
        return 0;
    }
}
