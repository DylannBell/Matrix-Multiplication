# MatrixMultiplication
A simple C program which performs matrix multiplication on two provided files which should be formatted according to the "matrix market" standard. 

# Directory Structure

* host -> host file detailing how many nodes to be run on the cluster
* m1.txt -> the text file generated by the program containing the first matrix ordered by row
* m2.txt -> the text file generated by the program containing the second matrix ordered by column
* Project3.c -> contains a simple program which multiplies two matricies in matrix market format
* Project3MPI.c -> contains a program which uses MPI + openMP to increase performance 
* Project3Report.docx -> a report detailing and analysing "Project3MPI.c"
* Statistics.xlsx -> contains the results obtain from "Project3MPI.c"
* test1.mtx -> a matrix in matrix market format for testing purposes
* test2.mtx -> a matrix in matrix market format for testing purposes
* test3.mtx -> a matrix in matrix market format for testing purposes
* test4.mtx -> a matrix in matrix market format for testing purposes
* test5.mtx -> a matrix in matrix market format for testing purposes

# Compiling & Running

* Project3.c
 ```
 gcc -o Project3 Project3.c
 ```
 ```
 ./Project3 test1.mtx test1.mtx
 ```
 
 * Project3MPI.c
 This file needs to run on some sort of cluster - the one used was provided by UWA,
 the following steps were used to compile and run the program
 ```
 mpicc -fopenmp -o Project3MPI Project3MPI.c
 ```
 ```
 syncCluster
 ```
 ```
 mpirun Projecr3MPI test5.mtx test5.mtx
 ```
 

# Notes

* The time given to complete this project (in my opinion) was not enough - there was alot of 3am finishes and 
the quality of this project isn't as high as I wanted it to be, however the core concepts on what we learnt throughout
the semester in terms of high performance computing are still there and I am still quite proud of the final product