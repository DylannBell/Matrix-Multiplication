#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	COMPILE : gcc -std=c99 -o Project3 Project3.c [fileName1] [fileName2] [fileSize1] [fileSize2]
	RUN [LINUX] : ./Project3.c test1.mtx test1.mtx 5 5
	RUN [WINDOWS] : Project3.c test1.mtx test1.mtx 5 5

	BOTH MATRIX FILES MUST BE ORDERED BY ROWS
*/

/*
	From a given file, stores each line into a 2-Dimensional matrix
	fp - a file pointer to the relevant file
	noOfRows - the number of rows in the file, computed elsewhere
	return - a 2D matrix with all the information from the file
*/

// I think a struct may be better so we can have ints and floats
// To access the values its just matrix1[i].row etc
struct SparseRow {
	int row;
	int col;
	float value;
};

void fileToMatrix(FILE *fp, int noOfRows, struct SparseRow *matrix)
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

	while (fgets(line, sizeof line, fp) != NULL )
	{	

		// Updated to NULL for 2nd and 3rd strok because that's how you get the next token
		matrix[lineNumber].row = atoi(strtok(line, " "));
		matrix[lineNumber].col = atoi(strtok(NULL, " "));
		matrix[lineNumber].value = atof(strtok(NULL, " "));
		
		printf("ROW : %d COLUMN : %d VALUE : %f \n", 
			matrix[lineNumber].row, matrix[lineNumber].col, matrix[lineNumber].value);

		lineNumber++;
	}
}

void main(int argc, char *argv[])
{
	//if the number of arguments is less than 3 - throw an error
	if(argc < 5)
	{
		printf("Error: you have not specified two files");
		exit(1);
	}
		
	//store file names within local variables
	char *file1 = argv[1];
	char *file2 = argv[2];
	
	//initialise an array - [fileSize1] [fileSize2]
	int noRows1 = atoi(argv[3]);
	int noRows2 = atoi(argv[4]);
	
	//open the files using fopen...
	FILE *fp1 = fopen (file1, "r");
	FILE *fp2 = fopen(file2, "r");
	
	// Create structs to hold sparse matrix
	struct SparseRow matrix1[noRows1];
	struct SparseRow matrix2[noRows2];

	// Load the file into the matrix
	fileToMatrix(fp1, noRows1, matrix1);
	fileToMatrix(fp2, noRows2, matrix1);	
	
	//close both files
	fclose(fp1);
	fclose(fp2);

	//http://www.mathcs.emory.edu/~cheung/Courses/561/Syllabus/3-C/sparse.html
	




}
