#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

/*
#define NRA 62 //number of rows in matrix A 
#define NCA 15 //number of columns in matrix A 
#define NCB 7 //number of columns in matrix B 
*/

#define MASTER 0 /* taskid of first task */
#define FROM_MASTER 1 /* setting a message type */
#define FROM_WORKER 2 /* setting a message type */
#define MAX_SIZE 1000

int ROWS;

struct SparseRow{
	int row;
	int col;
	float val;
};


/*
	From a given file name returns the number of lines within that file
	file - the file name as a string
	return - the number of lines within the specified file
*/
int countLines(char *file)
{
	FILE *fp = fopen(file, "r");
	char line[MAX_SIZE];

	if(fp == NULL){
		fprintf(stderr, "Error: cannot open file");
		exit(EXIT_FAILURE);
	}

	int numLines = 0;

    while(fgets(line, sizeof(line), fp) != NULL){
        numLines++;
    }

    return numLines;
}

/*
	From a given file, stores each line into a matrix of structs
	fp - a file pointer to the relevant file
*/
void fileToMatrix(FILE *fp, struct SparseRow *matrix)
{
	//if you can't open the file - throw an error
	if(fp == NULL)
	{
		printf("Error: cannot open file");
		exit(EXIT_FAILURE);
	}
	
	//for each of the lines within the file - split up the different components based on whitespace
	char line [ 128 ]; 
	int lineNumber = 0;

	while (fgets(line, sizeof line, fp) != NULL)
	{	
		matrix[lineNumber].row = atof(strtok(line, " "));
		matrix[lineNumber].col = atof(strtok(NULL, " ")) ;
		matrix[lineNumber].val = atof(strtok(NULL, " "));
		lineNumber++;
	}

}



void splitMatrices(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows) {	
	/*
	// Print offset values
	for (int i = 0; i < numWorkers; i++) {
		printf("Offset row = %d\n", offsetRow[i]);
	}
	for (int i = 0; i < numWorkers; i++) {
		printf("Offset col = %d\n", offsetCol[i]);
	}
	*/
}

int main (int argc, char *argv[])
{
	// WHERE THE FUCK DOES IT STORE THIS FILE.
	FILE *test = fopen("test.txt", "w");
	fprintf(test, "THIS IS A TEST\n");
	fclose(test);

	char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
    	printf("Current working dir: %s\n", cwd);
   	} else {
        perror("getcwd() error");
        return 1;
   	}

	// Don't think this will work if NZE < numWorkers
	int numWorkers = 11;
	int averageRow;
	int counter = 0;
	int currentRow;
	int currentCol;
	int i;


	int numtasks, /* number of tasks in partition */
	taskid, /* a task identifier */
	source, /* task id of message source */
	dest, /* task id of message destination */
	mtype, /* message type */
	rows,
	offset, /* used to determine rows sent
	to each worker */
	rc;

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
	numWorkers = numtasks-1;
	
	/****** master task *************/
	if (taskid == MASTER)
	{

		//if the number of arguments is less than 3 - throw an error
		if(argc < 3)
		{
			fprintf(stderr, "Error: you have not specified two files");
			exit(EXIT_FAILURE);
		}
			
		//store file names within local variables
		char *file1 = argv[1];
		char *file2 = argv[2];
		

		int m1NonZeroEntries = countLines(file1);
		int m2NonZeroEntries = countLines(file2);
		
		
		//sort the files using system()
		//sort -k1 -n test1.mtx > out
		//sort -k2 -n test2.mtx > out
		char *sortStart1 = "sort -k1 -n ";
		char *sortStart2 = "sort -k2 -n ";
		char *sortEnd1 = " > m1.txt";
		char *sortEnd2 = " > m2.txt";

		char *sortM1 = malloc(strlen(sortStart1) + strlen(file1) + strlen(sortEnd1) + 1);
		strcpy(sortM1, sortStart1);
		strcat(sortM1, file1);
		strcat(sortM1, sortEnd1);

		char *sortM2 = malloc(strlen(sortStart2) + strlen(file2) + strlen(sortEnd2) + 1);
		strcpy(sortM2, sortStart2);
		strcat(sortM2, file2);
		strcat(sortM2, sortEnd2);

		system(sortM1);
		system(sortM2);
		

		//open the files using fopen...
		FILE *fp1 = fopen ("m1.txt", "r");
		FILE *fp2 = fopen("m2.txt", "r");

		char line [ 128 ];

		if(fp1 == NULL || fp2 == NULL){
			printf("FILE failed\n");
			fprintf(stderr, "Error: cannot open file");
			exit(EXIT_FAILURE);
		}
		
		// Create the array to hold the structs
		struct SparseRow matrix1[m1NonZeroEntries];
		struct SparseRow matrix2[m2NonZeroEntries];

		// Load the file into the matrix
		fileToMatrix(fp1, matrix1);
		fileToMatrix(fp2, matrix2);	

		//callMPI(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, argc, argv);
		//sequentialMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, &result);

		//free any pointers which have used malloc
		free(sortM1);
		free(sortM2);

		//close both files
		fclose(fp1);
		fclose(fp2);

		printf("I AM MASTER\n");
		printf("mpi_mm has started with %d tasks.\n",numtasks);
		printf("Initializing arrays...\n");

		// Creating offset arrays
		int offsetRowValue[numWorkers];
		int offsetRow[numWorkers];
		int offsetCol[numWorkers];
		averageRow = m1NonZeroEntries / numWorkers;

		for (i = 1; i <= m1NonZeroEntries; i++) {
			currentRow = matrix1[i].row;
			if (i % averageRow == 0) {
				offsetRowValue[counter] = currentRow;
				counter++;
			}
		}

		counter = 0;
		for (i = 0; i < m1NonZeroEntries; i++) {
			currentRow = matrix1[i].row;
			if (currentRow <= offsetRowValue[counter]) {
				offsetRow[counter] = i;
			} else {
				counter++;
				offsetRow[counter+1] = i;
			}
		}
		
		counter = 0;
		for (i = 0; i < m2NonZeroEntries; i++) {
			currentCol = matrix2[i].col;
			if (currentCol <= offsetRowValue[counter]) {
				offsetCol[counter] = i;
			} else {
				counter++;
				offsetCol[counter+1] = i;
			}
		}

		
		// Print offset values
		for (i = 0; i < numWorkers; i++) {
			printf("Offset row = %d\n", offsetRow[i]);
		}
		for (i = 0; i < numWorkers; i++) {
			printf("Offset col = %d\n", offsetCol[i]);
		}


		mtype = FROM_MASTER;
		for(dest = 1; dest <= numWorkers; dest++)
		{
			MPI_Send(&offsetRow[dest-1], 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
		}

	} 

	// ****************************************** //

	if (taskid > MASTER)
	{
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		printf("IM IN A WORKER NODE WITH A OFFSET OF : %d\n", offset);
	}

	MPI_Finalize();
}
