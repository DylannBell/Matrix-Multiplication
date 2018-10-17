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
int resultRows;

struct SparseRow {
	int row;
	int col;
	float value;
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
		matrix[lineNumber].row = atof(strtok(line, " "))- 1;
		matrix[lineNumber].col = atof(strtok(NULL, " ")) -1;
		matrix[lineNumber].value = atof(strtok(NULL, " "));
		lineNumber++;
	}

}

/*
	Prints a given array in matrix market format to stdout
*/
void printMatrixMarketArray(float matrix[resultRows][3]) {

	for (int i = 0; i < resultRows; i++)
	{
		printf("%d ", matrix[i][0]);
		printf("%d ", matrix[i][1]);
		printf("%f ", matrix[i][2]);
		printf("\n");
	}
}

/*
	matrix1 - 
	matrix2 - 	
*/
void sequentialMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2, int m1Rows, int m2Rows) {

	//Create result matrix and initialise to 0.
	//HOW ARE WE MEANT TO KNOW WHAT THIS IS GOING TO BE BEFOREHAND?
	//DYNAMICALLY ALLOCATE?
	int ROWS = 3;
	int COLS = 3;
	resultRows = 3;
	float result[ROWS][COLS];
	memset(result, 0.0, sizeof result);

	// For each non zero entry in the first matrix.
	// Find entries such that m1Row = m2Col
	// Multiply result and add to result matrix
	// For parallelisation split up m1 by rows and send matching cols of m2?
	for (int i = 0; i < m1Rows; i++)
	{  
		//printf("FIRST LOOP - Row : %d Col : %d Val : %f \n", matrix1[i].row, matrix1[i].col, matrix1[i].value);
		for (int j = 0; j < m2Rows; j ++) 
		{
			//printf("SECOND LOOP - Row : %d Col : %d Val : %f \n", matrix2[j].row, matrix2[j].col, matrix2[j].value);
			if (matrix1[i].col == matrix2[j].row) 
			{
				int m1Row = matrix1[i].row;
				int m2Col = matrix2[j].col;
				float m1Val = matrix1[i].value;
				float m2Val = matrix2[j].value;
				result[m1Row][m2Col] += m1Val * m2Val;
			}
		}
	}

	printMatrixMarketArray(result);
	
	
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
	
	//open the files using fopen...
	FILE *fp1 = fopen (file1, "r");
	FILE *fp2 = fopen(file2, "r");
	
	// Create the array to hold the structs
	struct SparseRow matrix1[m1NonZeroEntries];
	struct SparseRow matrix2[m2NonZeroEntries];

	// Load the file into the matrix
	fileToMatrix(fp1, matrix1);
	fileToMatrix(fp2, matrix2);	
	
	sequentialMultiply(matrix1, matrix2, m1NonZeroEntries, m2NonZeroEntries);

	//close both files
	fclose(fp1);
	fclose(fp2);

	

	//https://www.geeksforgeeks.org/operations-sparse-matrices/
	//https://people.eecs.berkeley.edu/~aydin/spgemm_sisc12.pdf
	//http://mathforum.org/library/drmath/view/51903.html
	//https://toshitha.github.io/Parallel-Project/index.html
	

}