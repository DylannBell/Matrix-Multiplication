#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

/*
#define NRA 62 //number of rows in matrix A 
#define NCA 15 //number of columns in matrix A 
#define NCB 7 //number of columns in matrix B 
*/

#define MASTER 0 /* taskid of first task */
#define FROM_MASTER 1 /* setting a message type */
#define FROM_WORKER 2 /* setting a message type */

void callMPI() {
	int numtasks, /* number of tasks in partition */
	taskid, /* a task identifier */
	numworkers, /* number of worker tasks */
	source, /* task id of message source */
	dest, /* task id of message destination */
	mtype, /* message type */
	rows, /* rows of matrix A sent to each worker */
	averow, extra, offset, /* used to determine rows sent
	to each worker */
	i, j, k, rc; /* misc */

	double a[NRA][NCA], /* matrix A to be multiplied */
	b[NCA][NCB], /* matrix B to be multiplied */
	c[NRA][NCB]; /* result matrix C */
	MPI_Status status;


	//MPI_COMM_WORLD -> cluster
	//COMM_RANK -> current worker id
	//COMM_SIZE -> total number of workers in cluster
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

	if (numtasks < 2 ) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}
	numworkers = numtasks-1;
	
	/****** master task *************/
	if (taskid == MASTER)
	{
		printf("mpi_mm has started with %d tasks.\n",numtasks);
		printf("Initializing arrays...\n");


		//replace this with our reading from a file functionality 
		//loading it into a matrix of structs
		//-------------------------------------------------------
		for (i=0; i<NRA; i++)
			for (j=0; j<NCA; j++)
				a[i][j]= i+j;

		for (i=0; i<NCA; i++)
			for (j=0; j<NCB; j++)
				b[i][j]= i*j;
		//-------------------------------------------------------


		//roughly each node into n/number of workers where n is size of nonzero entries in m1
		/* Send matrix data to the worker tasks */
		averow = NRA/numworkers;
		extra = NRA%numworkers;
		offset = 0;
		mtype = FROM_MASTER;
		
		//for each worker..
		for (dest=1; dest<=numworkers; dest++)
		{
			//SENDING DATA TO WORKERS...
			rows = (dest <= extra) ? averow+1 : averow;
			printf("Sending %d rows to task %d offset=%d\n",
			rows,dest,offset);
			MPI_Send(&offset, 1, MPI_INT, dest,
			mtype, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest,
			mtype, MPI_COMM_WORLD);
			MPI_Send(&a[offset][0], rows*NCA, MPI_DOUBLE,
			dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&b, NCA*NCB, MPI_DOUBLE,
			dest, mtype, MPI_COMM_WORLD);
			offset = offset + rows;
		}

		//Receive results from worker tasks
		mtype = FROM_WORKER;
		for (i=1; i<=numworkers; i++)
		{
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, mtype,
			MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, mtype,
			MPI_COMM_WORLD, &status);
			MPI_Recv(&c[offset][0], rows*NCB, MPI_DOUBLE,
			source, mtype, MPI_COMM_WORLD, &status);
			printf("Received results from task %d\n",source);
		}

		/* Print results */
		printf("*******************\n");
		printf("Result Matrix:\n");
		for (i=0; i<NRA; i++)
		{
			printf("\n");
			for (j=0; j<NCB; j++)
				printf("%6.2f ", c[i][j]);
		}

		printf("\n*****************\n");
		printf ("Done.\n");
	}
	

	//WORKER/SLAVE NODE
	if (taskid > MASTER)
	{
		mtype = FROM_MASTER;
		//Recieve information...
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype,
		MPI_COMM_WORLD, &status);

		MPI_Recv(&rows, 1, MPI_INT, MASTER,
		mtype, MPI_COMM_WORLD, &status);

		MPI_Recv(&a, rows*NCA, MPI_DOUBLE, MASTER,
		mtype, MPI_COMM_WORLD, &status);

		MPI_Recv(&b, NCA*NCB, MPI_DOUBLE, MASTER, mtype,
		MPI_COMM_WORLD, &status);

		//dot product which we can do OPENMP on...
		for (k=0; k<NCB; k++) {
			for (i=0; i<rows; i++) {
				c[i][k] = 0.0;
				for (j=0; j<NCA; j++) {
					c[i][k] = c[i][k] + a[i][j] * b[j][k];
				}
			}
		}

		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype,
		MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype,
		MPI_COMM_WORLD);
		MPI_Send(&c, rows*NCB, MPI_DOUBLE, MASTER,
		mtype, MPI_COMM_WORLD);

	}

	MPI_Finalize();

}

int main (int argc, char *argv[])
{
	//initialise...
	callMPI();
}
