.SUFFIXES: -seq -omp
CC = icc

OPENMP	 = -fopenmp
CFLAGS   = -g -Wall -std=c99
CLIBS    = -lm

.c-seq: 
	$(CC) $(CFLAGS) $< -o $@ $(CLIBS) 

.c-omp: 
	$(CC) $(CFLAGS) $(OPENMP) $< -o $@ $(CLIBS) 

clean: 
	rm -rf *-seq *-omp 

ultraclean:
	rm -rf TRACE*.mpit *.prv *.pcf *.row set-0 *.mpits *.sym *.o* *.e* time-* strong-* *.ps *.txt
