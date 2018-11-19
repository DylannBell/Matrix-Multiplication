#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include<sys/time.h>

#define MAX_SIZE 1000

/*
	COMPILE : gcc -std=c99 -o Project3 Project3.c
	RUN [LINUX] : ./Project3 test1.mtx test1.mtx
	RUN [WINDOWS] : Project3 test1.mtx test1.mtx
*/

// Global variables
int* offsetRowArray;
int* offsetColArray;

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

/*
	Splits the matricies into partitions where each partition will be extended until the next number it encounters
	is different to the previous number it added
*/
void splitMatrices(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1NonZeroEntries, int m2NonZeroEntries) {

	int numWorkers = 11;

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

/*
	Sequentially multiplies two matricies together
*/
void sequentialMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows, struct SparseRow **result) {

	*result = malloc(1 * sizeof(struct SparseRow));

	//matrix multiplication with dot product
	int resultNonZeroEntries = 0;
	for(int i = 0; i < m1Rows; i++)
	{
		int curM1Row = matrix1[i].row;
		int curM1Col = matrix1[i].col;
		float curM1Value = matrix1[i].val;

		for(int j = 0; j < m2Rows; j++)
		{
			int curM2Row = matrix2[j].row;
			int curM2Col = matrix2[j].col;
			float curM2Value = matrix2[j].val;

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
}

/*
	Multiplies two matricies together - uses openMP to gain performance
*/
void matrixMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows, struct SparseRow **result) {
	
	#define thisThread omp_get_thread_num()
	#define nThreads omp_get_num_threads()

	// Shared variables
	int totalNonZero = 0;
	int *copyIndex;
	int *threadNonZero;

	#pragma omp parallel
	{
		// Each thread now initialize a local buffer and local variables 
		int localNonZero = 0;
		int allocatedSize = 1024;
		struct SparseRow *localResult;
		localResult = malloc(allocatedSize  * sizeof(struct SparseRow));

		// one thread initialize an array
		#pragma omp single
		{
			threadNonZero=malloc(nThreads*sizeof(int));
			copyIndex=malloc((nThreads+1)*sizeof(int));
		}

		/* 
	    realloc an extra 1024 lines each time localNonZeros exceeds allocatedSize
	    fill the local buffer and increment the localNonZeros counter
	    no need to use critical / atomic clauses
		*/
		#pragma omp for 
		for (int i = 0; i < m1Rows; i++){

			int curM1Col = matrix1[i].col;			
			for(int j = 0; j < m2Rows; j++)
			{

				int curM2Row = matrix2[j].row;
				
				if(curM1Col == curM2Row)
				{

					if (localNonZero >= allocatedSize) {
						allocatedSize += 1024;
						*result = realloc(*result,
						(sizeof(struct SparseRow)*(allocatedSize)));
					}

					int curM1Row = matrix1[i].row;
					float curM1Value = matrix1[i].val;
					int curM2Col = matrix2[j].col;
					float curM2Value = matrix2[j].val;

					localResult[localNonZero].row = curM1Row;
					localResult[localNonZero].col = curM2Col;
					localResult[localNonZero].val += curM1Value*curM2Value;
					localNonZero++;
				}
			}
		}

		// Put number of non zeri results into a shared result 
		threadNonZero[thisThread] = localNonZero; 
		#pragma omp barrier

		// Check how many non zero values for each thread, allocate the output and check where each thread will copy its local buffer
		#pragma omp single
		{
		    copyIndex[0]=0;
		    for (int i=0; i<nThreads; i++) {
		        copyIndex[i+1]=threadNonZero[i]+copyIndex[i];
		        //printf("Index %d = %d\n", i+1, copyIndex[i+1]);
		        totalNonZero += threadNonZero[i];
		    }

		    result = malloc( totalNonZero * sizeof(struct SparseRow) );
		}
		
		// Copy the results from local to global result
		memcpy(&result[copyIndex[thisThread]],localResult, localNonZero * sizeof(struct SparseRow));
		
		// Free memory
		free(localResult);

		#pragma omp single
		{
			free(copyIndex);
		}
	}
}

void main(int argc, char *argv[])
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
	char *sortEnd1 = " > m1.mtx";
	char *sortEnd2 = " > m2.mtx";

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
	FILE *fp1 = fopen ("m1.mtx", "r");
	FILE *fp2 = fopen("m2.mtx", "r");
	
	// Create the array to hold the structs
	struct SparseRow matrix1[m1NonZeroEntries];
	struct SparseRow matrix2[m2NonZeroEntries];

	// Load the file into the matrix
	fileToMatrix(fp1, matrix1);
	fileToMatrix(fp2, matrix2);	

	splitMatrices(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries);

	struct SparseRow *result;

	// TIME FUNCTION
	struct timeval start, end, start1, end1;

	//SEQUENTIAL 1
	gettimeofday(&start, NULL);

	sequentialMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, &result);

	gettimeofday(&end, NULL);

	pMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries);


	float parallel_time_1 = ((end.tv_sec  - start.tv_sec) * 1000000u +
    end.tv_usec - start.tv_usec) / 1.e6;

    printf("Sequential Time = %12.7f\n", parallel_time_1);

    //PARALLEL 1
    struct SparseRow *result1;
	gettimeofday(&start1, NULL);

	matrixMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, &result1);

	gettimeofday(&end1, NULL);


	float parallel_time_2 = ((end1.tv_sec  - start1.tv_sec) * 1000000u +
    end1.tv_usec - start1.tv_usec) / 1.e6;

    printf("P Time = %12.7f\n", parallel_time_2);
	
	//free any pointers which have used malloc
	free(sortM1);
	free(sortM2);

	//close both files
	fclose(fp1);
	fclose(fp2);
}