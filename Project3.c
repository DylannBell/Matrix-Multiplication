#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SIZE 1000

/*
	COMPILE : gcc -std=c99 -o Project3 Project3.c
	RUN [LINUX] : ./Project3 test1.mtx test1.mtx
	RUN [WINDOWS] : Project3 test1.mtx test1.mtx
*/


// Global variables
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

/*
	Prints a given array in matrix market format to stdout
*/
void printMatrixMarketArray(struct SparseRow *matrix) {

	printf("RESULT MATRIX\n");
	for (int i = 0; i < ROWS; i++)
	{
		printf("%i ", matrix[i].row);
		printf("%i ", matrix[i].col);
		printf("%f ", matrix[i].val);
		printf("\n");
	}
}

/*
	matrix1 - 
	matrix2 - 	
*/
void sequentialMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows, struct SparseRow **result) {
	
	printf("MATRIX 1 \n");
	for(int i = 0; i < m1Rows; i++)
	{
		printf("%i %i %f \n", matrix1[i].row, matrix1[i].col, matrix1[i].val);
	}
	printf("\n");
	
	printf("MATRIX 2 \n");
	for(int i = 0; i < m2Rows; i++)
	{
		printf("%i %i %f \n", matrix2[i].row, matrix2[i].col, matrix2[i].val);
	}
	printf("\n");
	

	//this "ROWS" variable shouldn't be hardcoded...
	ROWS = 5;
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

			if((curM1Row == curM2Col) && (curM1Col == curM2Row))
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

	printf("RESULT MATRIX\n");
	for (int i = 0; i < resultNonZeroEntries; i++)
	{
		printf("%d ", (*result)[i].row);
		printf("%d ", (*result)[i].col);
		printf("%f ", (*result)[i].val);
		printf("\n");
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
	
	struct SparseRow *result = NULL;
	sequentialMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries, &result);

	//free any pointers which have used malloc
	free(sortM1);
	free(sortM2);

	//close both files
	fclose(fp1);
	fclose(fp2);

	//https://www.geeksforgeeks.org/operations-sparse-matrices/
	//https://people.eecs.berkeley.edu/~aydin/spgemm_sisc12.pdf
	//http://mathforum.org/library/drmath/view/51903.html
	//https://toshitha.github.io/Parallel-Project/index.html
	

}