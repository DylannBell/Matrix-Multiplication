#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
	COMPILE : gcc -std=c99 -o Project3 Project3.c [fileName1] [fileName2] [fileSize1] [fileSize2]
	RUN [LINUX] : ./Project3.c test1.mtx test1.mtx
	RUN [WINDOWS] : Project3.c test1.mtx test1.mtx
	BOTH MATRIX FILES MUST BE ORDERED BY ROWS
*/


// Global variables
int ROWS;
int COLS;
int m1NonZeroEntries;
int m2NonZeroEntries;

struct SparseRow {
	int row;
	int col;
	float value;
};

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
		matrix[lineNumber].row = atoi(strtok(line, " "))-1;
		matrix[lineNumber].col = atoi(strtok(NULL, " "))-1;
		matrix[lineNumber].value = atof(strtok(NULL, " "));
		
		printf("Row: %d, Column : %d, Value: %f \n", matrix[lineNumber].row, matrix[lineNumber].col, matrix[lineNumber].value);

		lineNumber++;
	}
}


/*
	Prints a given array to stdout
*/
void printArray(float array[ROWS][ROWS]) {
	for (int i = 0; i < ROWS; i++) 
	{
		for (int j = 0; j < COLS; j++) 
		{
			printf("%f ", array[i][j]);
		}
		printf("\n");
	}
}


void sequentialMultiply(struct SparseRow *matrix1, struct SparseRow *matrix2) {

	int m1NonZeroEntries = 4;
	int m2NonZeroEntries = 4;

	ROWS = 3;
	COLS = 3;

	//Create result matrix and initialise to 0.
	float result[ROWS][COLS];
	memset(result, 0.0, sizeof result);


	// For each non zero entry in the first matrix.
	// Find entries such that m1Row = m2Col
	// Multiply result and add to result matrix
	// For parallelisation split up m1 by rows and send matching cols of m2?

	for (int i = 0; i < m1NonZeroEntries; i++)
	{  
		for (int j = 0; j < m2NonZeroEntries; j ++) 
		{
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
	printArray(result);
}


void main(int argc, char *argv[])
{
	//if the number of arguments is less than 3 - throw an error
	if(argc < 5)
	{
		printf("Error: you have not specified two files");
		exit(EXIT_FAILURE);
	}
		
	//store file names within local variables
	char *file1 = argv[1];
	char *file2 = argv[2];
	
	//initialise an array - [Number of Rows] [Number of Cols]


	ROWS = atoi(argv[3]);
	COLS = atoi(argv[4]);

	
	//open the files using fopen...
	FILE *fp1 = fopen (file1, "r");
	FILE *fp2 = fopen(file2, "r");
	
	// Create structs to hold sparse matrix
	m1NonZeroEntries = 4;
	m2NonZeroEntries = 4;

	// Create the array to hold the structs
	struct SparseRow matrix1[m1NonZeroEntries];
	struct SparseRow matrix2[m2NonZeroEntries];

	// Load the file into the matrix
	fileToMatrix(fp1, matrix1);
	fileToMatrix(fp2, matrix2);	
	

	//close both files
	fclose(fp1);
	fclose(fp2);

	sequentialMultiply(matrix1, matrix2);


	//https://www.geeksforgeeks.org/operations-sparse-matrices/
	//https://people.eecs.berkeley.edu/~aydin/spgemm_sisc12.pdf
	//http://mathforum.org/library/drmath/view/51903.html
	//https://toshitha.github.io/Parallel-Project/index.html
	

}