/*************************************************************
*
* Gaussian Elimination
*
* Pthreads Parallel Version
*
* File: gaussian_par.c
* Author(s): Bala Jaswanth, Haritha Reddy
*
* Blekinge Institute of Technology
* 
*************************************************************/

#include <stdio.h>
#include <pthread.h>

#define MAX_SIZE 4096
//#define DEFAULT_NUM_CPUS 8	
#define NUM_CPUS 8

typedef double matrix[MAX_SIZE][MAX_SIZE];

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char	*Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
double	b[MAX_SIZE];	/* vector b             */
double	y[MAX_SIZE];	/* vector y             */
//int NUM_CPUS;	/* number of cpus */
pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void
Init_Matrix()
{
    int i, j;
 
    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");
 
    if (strcmp(Init,"rand") == 0) {
	for (i = 0; i < N; i++){
	    for (j = 0; j < N; j++) {
		if (i == j) /* diagonal dominance */
		    A[i][j] = (double)(rand() % maxnum) + 5.0;
		else
		    A[i][j] = (double)(rand() % maxnum) + 1.0;
	    }
	}
    }
    if (strcmp(Init,"fast") == 0) {
	for (i = 0; i < N; i++) {
	    for (j = 0; j < N; j++) {
		if (i == j) /* diagonal dominance */
		    A[i][j] = 5.0;
		else
		    A[i][j] = 2.0;
	    }
	}
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++) {
	b[i] = 2.0;
	y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
	Print_Matrix();
}

void
Print_Matrix()
{
    int i, j;
 
    printf("Matrix A:\n");
    for (i = 0; i < N; i++) {
	printf("[");
	for (j = 0; j < N; j++)
	    printf(" %5.2f,", A[i][j]);
	printf("]\n");
    }
    printf("Vector b:\n[");
    for (j = 0; j < N; j++)
	printf(" %5.2f,", b[j]);
    printf("]\n");
    printf("Vector y:\n[");
    for (j = 0; j < N; j++)
	printf(" %5.2f,", y[j]);
    printf("]\n");
    printf("\n\n");
}

void 
Init_Default()
{
    N = 2048; 
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
	//NUM_CPUS = DEFAULT_NUM_CPUS;
}
 
int
Read_Options(int argc, char **argv)
{
    char    *prog;
 
    prog = *argv;
    while (++argv, --argc > 0)
	if (**argv == '-')
	    switch ( *++*argv ) {
	    case 'n':
		--argc;
		N = atoi(*++argv);
		break;
	    case 'h':
		printf("\nHELP: try sor -u \n\n");
		exit(0);
		break;
	    case 'u':
		printf("\nUsage: sor [-n problemsize]\n");
		printf("           [-D] show default values \n");
		printf("           [-h] help \n");
		printf("           [-I init_type] fast/rand \n");
		printf("           [-m maxnum] max random no \n");
		printf("           [-P print_switch] 0/1 \n");
		//printf("           [-C number of CPUs] 0/8 \n");
		exit(0);
		break;
	    case 'D':
		printf("\nDefault:  n         = %d ", N);
		printf("\n          Init      = rand" );
		printf("\n          maxnum    = 5 ");
		printf("\n          P         = 0 \n\n");
		//printf("\n          CPUs       = %d \n\n", NUM_CPUS);
		exit(0);
		break;
	    case 'I':
		--argc;
		Init = *++argv;
		break;
	    case 'm':
		--argc;
		maxnum = atoi(*++argv);
		break;
	    case 'P':
		--argc;
		PRINT = atoi(*++argv);
		break;
		/*case 'C':
		--argc;
		NUM_CPUS = atoi(*++argv);
		break;*/
	    default:
		printf("%s: ignored option: -%s\n", prog, *argv);
		printf("HELP: try %s -u \n\n", prog);
		break;
	    } 
}

void
work(void *thread_Id)
{
    int i, j, k;
    long thread_id = (long)thread_Id;

	pthread_mutex_init(&mutex,NULL);

    /* Gaussian elimination algorithm */
    for (k = 0; k < N; k++) { /* Outer loop */

       if(thread_id == (k % NUM_CPUS)) /* Allocating the rows to the threads in division group */
       {
		   //pthread_mutex_lock(&mutex);
          for (j = k+1; j < N ; j++)
		  {
			 pthread_mutex_lock(&mutex);
			 A[k][j] = A[k][j] / A[k][k]; /* Division step */
			 pthread_mutex_unlock(&mutex);
		  }
          y[k] = b[k] / A[k][k];
          
		  pthread_mutex_lock(&mutex);
		  A[k][k] = 1.0;
		  pthread_mutex_unlock(&mutex);
       }

       int bw = pthread_barrier_wait(&barrier); /* waiting for other threads to synchronize at this barrier */
	   if(bw != 0 && bw != PTHREAD_BARRIER_SERIAL_THREAD)
	   {
		   printf("Could not wait on barrier \n");
		   exit(-1);
	   }

       for (i = k+1; i < N ; i++) {
          if(thread_id == (i % NUM_CPUS)) /* Allocating the rows to the threads in elimination group */
          {

	     for (j = k+1; j < N ; j++)
	        A[i][j] = A[i][j] - A[i][k]*A[k][j]; /* Elimination step */

			 b[i] = b[i] - A[i][k]*y[k];
			
			A[i][k] = 0.0;
          }
       }

       int bw1 = pthread_barrier_wait(&barrier); /* waiting for other threads to synchronize at this barrier */
	   if(bw1 != 0 && bw1 != PTHREAD_BARRIER_SERIAL_THREAD)
	   {
		   printf("Could not wait on barrier \n");
		   exit(-1);
	   }
    }
}

int 
main(int argc, char **argv)
{
    pthread_t threads[NUM_CPUS];
    long i;
 
    Init_Default();		/* Init default values	*/
    Read_Options(argc,argv);	/* Read arguments	*/
    Init_Matrix();		/* Init the matrix	*/

    //pthread_mutex_init(&mutex, NULL)
    if(pthread_barrier_init(&barrier, NULL, NUM_CPUS)) /* Initializing a synchronization barrier */
	{
		/* On success pthread_barrier_init returns 0  */
		printf("Could not initialize the barrier \n");
		return -1;
	}


    for(i = 0; i < NUM_CPUS; i++)
	{
      if(pthread_create (&threads[i], NULL, (void *) &work, (void *) i)) /* Creating new threads */
	  {
		  /* On success pthread_create returns 0  */
		  printf("Could not create thread \n");
		  return -1;
	  }
	}

    for(i = 0; i < NUM_CPUS; i++)
	{
      if(pthread_join(threads[i],NULL))  /* Suspending execution of the calling thread */
		{
			/* On success pthread_join returns 0  */
			printf("Could not join thread \n");
			return -1;
		}
	}

	//pthread_mutex_destroy(&mutex)
    if(pthread_barrier_destroy(&barrier))  /* Destroying the barrier */
	{
		/* On success pthread_destroy returns 0  */
		printf("Barrier could not be destroyed");
		return -1;
	}

    if (PRINT == 1)
	Print_Matrix();
}
