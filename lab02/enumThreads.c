/**

   This program overwrites the file "foo", writing the integer 1 on
   the first line.

   Then the program iterates N times, specified by a command line argument.
   On each iteration:
   
   1. The program positions the read marker at the beginning of the file.

   2. It then reads all integers from a file, each on a line, verifying that
   the integers are in ascending order with a difference of one
   between successive lines.  If an error is found, the program stops looping
   and reports where the error occurred.

   3. If no error is found, the program adds one more integer to the
   integers in the file.

*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define PRIMES_LIMIT 10000 // upper bound for primes search.
// tune this to force each thread to use a significant fraction of its quantum.

#define ORDER_ERROR  -1
int Nthreads;

typedef struct threadArgsStruct {
  int nreps;
  FILE *fp;
  int threadID;
} ThreadArgsT;

/*
  PRE
    limit: largest number to inspect
    printmod: how often to print the most recent prime
  This is a computation waster.  Feel free to provid a better algorithm!
 */
void getprimes(long limit, long printmod) {
  long i,j;
  long recentprime = 1;
  long numprimes = 0;
  /* loop to limit */
  for (i = 2; i <= limit; i++) {
    /* try all numbers from 2 to limit-1 */
    for (j = 2; j < i; j++) {
      if (i%j == 0) { /* not prime */
        break;
      }
    }
    if (j == i) { /* made it to end of loop => number was prime */
      recentprime = i;
      numprimes++;
      /* print out every printmod number of primes */
      if ( printmod != 0 && (numprimes%printmod) == 0) {
        printf("%10ld\t%10ld\n",numprimes,recentprime);
      }

    }
  }
}


/*
  Validate current file of integers.  Returns negative value for error
  or the value of the largest integer read.
  PRE: file handle fp open for reading.
  PST: reads fp to EOF
*/
int verifyOrder(FILE *fp) {

  int value;
  int oldvalue;

  rewind(fp); // rewind stream

  // Get initial value, error if there isn't one.
  if (!feof(fp)) {
    fscanf(fp,"%d\n",&oldvalue);
    // printf("old %d new %d\n",oldvalue, value);

  } else { 
    return ORDER_ERROR; 
  }
  if (oldvalue != 1) {
    return ORDER_ERROR;
  }
  // Now check all subsequent lines.
  while (!feof(fp)) {
    fscanf(fp,"%d\n",&value);
    //    printf("old %d new %d\n",oldvalue, value);
    if (value != (oldvalue + 1) ) {
      return ORDER_ERROR;
    }
    oldvalue = value;
  }

  return oldvalue; // no error so return last read value
}

/*
  See to end of file and write num on a new line.
  PRE: fp open for writing.
  PST: num appended to fp.
*/
void addLine(FILE *fp, int num) {
  fseek(fp,0L,SEEK_END); // seek to end of file to add next line  
  fprintf(fp,"%d\n",num);  
  fflush(fp);
}

/**
   Function run to complete the task.  

   PRE: arg contains a ThreadArgsT that contains the file pointer for I/O
   and the termination value, nreps.
*/
void *run_enum(void *arg) {
  int count;
  int status;

  ThreadArgsT *args = (ThreadArgsT *) arg;
  int nreps = args->nreps;
  FILE *fp;
  fp = args->fp;
  
  count = 2; // starts after 1 already written
  while (count < nreps) {
    getprimes(PRIMES_LIMIT,0); // waste some time
     status = verifyOrder(fp); 

    if (status == ORDER_ERROR) {
      fprintf(stderr,"Error at number %d\n",count);
      break;
    }
    count = status + 1; // status has last number read, add 1
    addLine(fp,count);  // output count and newline
  }
  pthread_exit(&nreps);
}


int main(int argc, char *argv[]) {
  FILE *fp;
  int count = 1;
  int i;
  pthread_t *thread_ID;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // Process args (not very robust)
  if (argc != 3) {
    fprintf(stderr,"USAGE: enum NUM NTHREADS\n");
    fprintf(stderr,"Writes to file foo from 1 to NUM using NTHREADS threads.\n");
    exit(EXIT_FAILURE);
  }
  int nreps = atoi(argv[1]); // convert ascii to int
  int Nthreads = atoi(argv[2]); // ignored at present
  
  thread_ID = (pthread_t *) malloc(sizeof(pthread_t)*Nthreads);
  
  fp = fopen("foo","w+"); // open/truncate file writing AND reading
  fprintf(fp,"%d\n",count); // write 1 on first line
  fflush(fp); // ensure that data is ouput to file.

  // Package args in structure for use when pthreads are used.
  ThreadArgsT args;
  args.nreps = nreps;
  args.fp = fp;
  
  //run_enum(&args);

  for (i = 0; i < Nthreads; i++) {
    pthread_create(&(thread_ID[i]), NULL, run_enum,&args);
    args.threadID = i;
  }
  pthread_join(*thread_ID,NULL);
  fclose(fp); // close file
  free(thread_ID);
  return(0);
}

