#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	COMPILE : gcc -std=c99 -o Project3 Project3.c [fileName1] [fileName2] [fileSize1] [fileSize2]
	RUN [LINUX] : ./Project3.c test1.txt test2.txt 10 10
	RUN [WINDOWS] : Project3.c test1.txt test2.txt 10 10
*/

/*
	From a given file, stores each line into a 2-Dimensional matrix
	fp - a file pointer to the relevant file
	noOfRows - the number of rows in the file, computed elsewhere
	return - a 2D matrix with all the information from the file
*/
void fileToMatrix(FILE *fp, int noOfRows, float** matrix)
{
	//if you can't open the file - throw an error
	if(fp == NULL)
	{
		printf("Error: cannot open file");
		exit(1);
	}
	
	//for each of the lines within the file - split up the different components based on whitespace
	char line [ 128 ]; 
	int row = 0;
	while (fgets(line, sizeof line, fp) != NULL )
	{	
		matrix[row][1] = atof(strtok(line, " "));
		matrix[row][2] = atof(strtok(line, " "));
		matrix[row][3] = atof(strtok(line, " "));
		
		printf("ROW : %f COLUMN : %f VALUE : %f \n", matrix[row][1], matrix[row][2], matrix[row][3]);
		
		row++;
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
	
	/**
	
		https://stackoverflow.com/questions/3911400/how-to-pass-2d-array-matrix-in-a-function-in-c
	
	**/
	
	//store the file values into a matrix
	float **matrix1 = malloc(noRows1 * sizeof *matrix1);
	for(int i = 0; i < noRows1; i++)
	{
		matrix1[i] = malloc(3 * sizeof *matrix1[i]);
	}
	fileToMatrix(fp1, noRows1, matrix1);
	
	float **matrix2 = malloc(noRows2 * sizeof *matrix2);
	for(int j = 0; j < noRows2; j++)
	{
		matrix2[j] = malloc(3 * sizeof *matrix2[j]);
	}
	fileToMatrix(fp1, noRows2, matrix2);
	
	
	//deallocate memory for both arrays
	for (int i=0; i<noRows1; i++)
	{
		free(matrix1[i]);
	}
	free(matrix1);
	
	for (int i=0; i<noRows2; i++)
	{
		free(matrix2[i]);
	}
	free(matrix2);
	
	//close both files
	fclose(fp1);
	fclose(fp2);
	




}