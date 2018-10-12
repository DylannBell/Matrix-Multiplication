#include <stdio.h>
#include <stdlib.h>

/*
	COMPILE : gcc -o Project3 Project3.c [fileName1] [fileName2]
	RUN [LINUX] : ./Project3.c test1.txt test2.txt
	RUN [WINDOWS] : Project3.c test1.txt test2.txt
*/

void main(int argc, char *argv[])
{
	//if the number of arguments is less than 3 - throw an error
	if(argc < 3)
	{
		printf("Error: you have not specified two files");
		exit(1);
	}
	
	//store file names within local variables
	char *file1 = argv[1];
	char *file2 = argv[2];
	
	//open the files using fopen...
	FILE *fp1 = fopen (file1, "r");
	FILE *fp2 = fopen(file2, "r");
	
	//if you can't open the file throw an error
	if(fp1 == NULL || fp2 == NULL)
	{
		printf("Error: cannot open file");
		exit(1);
	}

	//print off file1
	printf("FILE 1 \n");
	
	char line1 [ 128 ]; 
	while (fgets(line1, sizeof line1, fp1) != NULL )
	{
		fputs ( line1, stdout );
	}

	//print off file2
	printf("FILE 2 \n");
	
	char line2 [ 128 ]; 
	while (fgets(line2, sizeof line2, fp2) != NULL )
	{
		fputs ( line2, stdout );
	}
	
	//close both files
	fclose(fp1);
	fclose(fp2);
	
	
	
	//sort out each of the files

	//for each of the file names given

		//open the file with this file name

		//load this file...

		//sort this file using the linux shell command sort

		//write this to a file - matrix1 and matrix2

	// do your fancy stuff...




}