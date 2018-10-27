#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h> //cwd


#define MASTER 0 /* taskid of first task */
#define FROM_MASTER 1 /* setting a message type */
#define FROM_WORKER 2 /* setting a message type */
#define MAX_SIZE 1000

int* offsetRowArray;
int* offsetColArray;

typedef struct SparseRow {
	int row;
	int col;
	float val;
} sparse_row;


/*
	From a given file name returns the number of lines within that file
	file - the file name as a string
	return - the number of lines within the specified file
*/
int countLines(char *file) {
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
void fileToMatrix(FILE *fp, struct SparseRow *matrix) {
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

/*
	Multiplies two matricies in market matrix format together 
	matrix1
	matrix2
	m1Rows - the number of rows matrix1 contains
	m2Rows - the number of rows matrix2 contains
	result - the matrix which contains the result of the matrix multiplication
*/
int sequentialMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows, struct SparseRow **result) {

	*result = malloc(1 * sizeof(struct SparseRow));

	//matrix multiplication with dot product
	int resultNonZeroEntries = 0;
	int i, j;
	for(i = 0; i < m1Rows; i++)
	{
		int curM1Row = matrix1[i].row;
		int curM1Col = matrix1[i].col;
		float curM1Value = matrix1[i].val;

		for(j = 0; j < m2Rows; j++)
		{
			int curM2Row = matrix2[j].row;
			int curM2Col = matrix2[j].col;
			float curM2Value = matrix2[j].val;

			//if((curM1Row == curM2Col) && (curM1Col == curM2Row))
			if(curM1Col == curM2Row)
			{
				if(resultNonZeroEntries!= 0)
				{
					*result = realloc(*result, (sizeof(struct SparseRow)*(resultNonZeroEntries+1)));
				}

				(*result)[resultNonZeroEntries].row = curM1Row;
				(*result)[resultNonZeroEntries].col = curM2Col;
				(*result)[resultNonZeroEntries].val += curM1Value*curM2Value;
				resultNonZeroEntries++;
				break;
			}

		}
	}

	/*
	printf("RESULT MATRIX\n");
	for (int i = 0; i < resultNonZeroEntries; i++)
	{
		printf("%d ", (*result)[i].row);
		printf("%d ", (*result)[i].col);
		printf("%f ", (*result)[i].val);
		printf("\n");
	}
	*/
	return resultNonZeroEntries;
}


/*
	Partitions matrix 1 and matrix 2 according to rows and columns respectively 
*/
void splitMatrices(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1NonZeroEntries, int m2NonZeroEntries, int numWorkers) {

	int currentRow;
	int currentCol;
	int averageRow = m1NonZeroEntries / numWorkers;
	int i;
	int counter;

	// Arrays
	int offsetValue[numWorkers];
	int indexArray[numWorkers];

	offsetRowArray = malloc((numWorkers) * sizeof(offsetRowArray));
	offsetColArray = malloc((numWorkers) * sizeof(offsetRowArray));

	counter = 0;
	int lastRow = -1;

	// ROW
	for (i = 0; i < m1NonZeroEntries; i++) {
		currentRow = matrix1[i].row;
		if (currentRow == lastRow) {
			offsetRowArray[counter-1] = i;
			offsetValue[counter-1] = currentRow;

		}
		else if ((i+1) % averageRow == 0) {
			lastRow = matrix1[i].row;
			offsetRowArray[counter] = i;
			offsetValue[counter] = currentRow;
			counter++;
		}
	}

	//COL
	counter = 0;
	for (i = 0; i < m2NonZeroEntries; i++) {
		currentCol = matrix2[i].col;
		currentRow = offsetValue[counter];
		if (currentCol <= currentRow) {
			offsetColArray[counter] = i;
		} else {
			counter++;
			offsetColArray[counter] = i;
		}
	}

}



int main (int argc, char *argv[]) {

	//ASSUMPTION : the number of unique numbered rows is greater then the number of workers
	
	int i, //loop index
	numWorkers, // records the number of workers in the environment
	numtasks, // number of tasks in partition
	taskid, // a task identifier
	source, // task id of message source 
	dest, // task id of message destination
	mtype, // message type 
	offset, // used to determine rows sent to each worker  
	rc, //error code for MPI_abort
	m1Rows, //number of rows matrix 1 contains
	m2Rows; //number of rows matrix 2 contains
	MPI_Status status; //will represent the status of any message

	//initialise a MPI "session"
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid); 
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

	if (numtasks < 2 ) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}

	//initialise the number of workers in the environment
	numWorkers = numtasks-1;
	
	//create MPI_datatype for an array of structs
	int nItems = 3;
	int blocklenghts[3] = {1,1,1};
	MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_FLOAT};
	MPI_Datatype mpi_struct;
	MPI_Aint offsets[3];
	offsets[0] = offsetof(sparse_row, row);
	offsets[1] = offsetof(sparse_row, col);
	offsets[2] = offsetof(sparse_row, val);
	MPI_Type_create_struct(nItems, blocklenghts, offsets, types, &mpi_struct);
	MPI_Type_commit(&mpi_struct);

	//if we are at the MASTER node
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
		
		//create the array to hold the structs
		struct SparseRow matrix1[m1NonZeroEntries];
		struct SparseRow matrix2[m2NonZeroEntries];

		//load the file into the matrix
		fileToMatrix(fp1, matrix1);
		fileToMatrix(fp2, matrix2);	

		//free any pointers which have used malloc
		free(sortM1);
		free(sortM2);

		//close both files
		fclose(fp1);
		fclose(fp2);

		// plit matrix1 by rows and matrix2 by corresponding cols
		splitMatrices(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, numWorkers);
		
		//sending data to worker nodes
		mtype = FROM_MASTER;
		int rowsToSend;

		//for each worker node we know within the environment
		for(dest = 1; dest <= numWorkers; dest++)
		{
			//partition the data based on row and column
			int sizeOfPartitionRow = 0;
			int sizeOfPartitionCol = 0;
			int partitionRowIndex = 0;
			int partitionColIndex = 0;
			int j = dest - 1;

			//determines the size of the partitions
			if(j == 0) {
				sizeOfPartitionRow = offsetRowArray[j] + 1;
				sizeOfPartitionCol = offsetColArray[j] + 1;
			} 
			else if((j+1) == numWorkers) {
				sizeOfPartitionRow = m1NonZeroEntries - offsetRowArray[j];
				sizeOfPartitionCol = m2NonZeroEntries - offsetColArray[j];
			}
			else {
				sizeOfPartitionRow = offsetRowArray[j] - offsetRowArray[j - 1];
				sizeOfPartitionCol = offsetColArray[j] - offsetColArray[j - 1];
			}

			if((j+1) != numWorkers) {
				partitionRowIndex = offsetRowArray[j] - sizeOfPartitionRow + 1;			
				partitionColIndex = offsetColArray[j] - sizeOfPartitionCol + 1;				
			}
			else {
				partitionRowIndex = offsetRowArray[j];
				partitionColIndex = offsetColArray[j];
			}

			/*
			printf("----------------------------------\n");
			printf("WORKER : %d\n", dest);
			printf("Size Of Partition Row : %d\n", sizeOfPartitionRow);
			printf("Starting Index Of Row : %d\n", partitionRowIndex);
			printf("Size Of Partition Col : %d\n", sizeOfPartitionCol);
			printf("Starting Index Of Col : %d\n", partitionColIndex);
			*/
			
			//sending information to the workers
			MPI_Send(&sizeOfPartitionRow, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&sizeOfPartitionCol, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&matrix1[partitionRowIndex], sizeOfPartitionRow, mpi_struct, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&matrix2[partitionColIndex], sizeOfPartitionCol, mpi_struct, dest, mtype, MPI_COMM_WORLD);
			
		}
	} 

	if (taskid > MASTER)
	{
		mtype = FROM_MASTER;
		int sizeOfPartitionRow;
		int sizeOfPartitionCol;
		
		MPI_Recv(&sizeOfPartitionRow, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&sizeOfPartitionCol, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);

		struct SparseRow matrix1Part[sizeOfPartitionRow];
		struct SparseRow matrix2Part[sizeOfPartitionCol];

		MPI_Recv(&matrix1Part, sizeOfPartitionRow, mpi_struct, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&matrix2Part, sizeOfPartitionCol, mpi_struct, MASTER, mtype, MPI_COMM_WORLD, &status);

		/*
		printf("----------------------------------\n");
		printf("WORKER : %d\n", taskid);
		printf("ROW PARTITION : \n");
		for(i = 0; i < sizeOfPartitionRow; i++) {
			printf("%d %d %f \n", matrix1Part[i].row, matrix1Part[i].col, matrix1Part[i].val);
		}
		printf("COL PARTITION : \n");
		for(i = 0; i < sizeOfPartitionCol; i++) {
			printf("%d %d %f \n", matrix2Part[i].row, matrix2Part[i].col, matrix2Part[i].val);
		}
		*/

		/*
		int resultRows;
		struct SparseRow *result = NULL;
		resultRows = sequentialMultiply(matrix1Part, matrix2Part, m1Rows, m2Rows, &result);

		for (i = 0; i < resultRows; i++) {
			//printf("Result = %d %d %f\n", result[i].row, result[i].col, result[i].val);
		}
		*/
		

	}

	MPI_Finalize();
}
