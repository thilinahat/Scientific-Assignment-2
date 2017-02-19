//
// Created by thilina on 2/19/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for clock_gettime( )
#include <errno.h> // for perror( )

#define GET_TIME(x) if (clock_gettime(CLOCK_MONOTONIC, &(x)) < 0) \
{ perror("clock_gettime( ):"); exit(EXIT_FAILURE); }

void print2DArray(int *array, int size){

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            printf(" %2d", *((array+i*size)+j));
        }
        printf("\n");
    }
}

void print1DArray(int *array, int size){

    for (int j = 0; j < size; ++j) {
        printf(" %d", *array++);
    }
}

void generateRandomSparseMat(int *mat, int size){

}

void convertToCRS(int *mat, int *values, int *columnIndexes, int *rowPointers, int size){

    int count = 0;
    int i,j;
    int rowPointerFound = 0;

    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            if(*((mat+i*size)+j) > 0){
                *values++ = *((mat+i*size)+j);
                *columnIndexes++ = j+1;
                if(!rowPointerFound){
                    *rowPointers++ = count+1;
                    rowPointerFound = 1;
                }
                count++;
            }
        }
        if(!rowPointerFound) // complete zero elements row
            *rowPointers++ = -1;
        rowPointerFound = 0;
    }
    *rowPointers++ = count+1; // end of columnIndexes
}

void sparseMatrixAddition(int *matAValues, int *matAColumnIndexes, int *matARowPointers, int matANNZ,
                          int *matBValues, int *matBColumnIndexes, int *matBRowPointers, int matBNNZ,
                          int *matCValues, int *matCColumnIndexes, int *matCRowPointers, int *matCNNZ, int size){

    int matARowBeginIndex, matBRowBeginIndex,
            matAOffset, matBOffset,
            matAColumnIndex, matBColumnIndex,
            matANextRowBeginIndex, matBNextRowBeginIndex,
            matAOffsetToNextRow, matBOffsetToNextRow;

    int k, priorMatCNNZ;

    for (int i = 0; i < size; ++i) {
        matARowBeginIndex = *(matARowPointers+i);
        matBRowBeginIndex = *(matBRowPointers+i);
        priorMatCNNZ = *matCNNZ;

        if(matARowBeginIndex == -1 && matBRowBeginIndex == -1){ // both matrices have complete zero rows
            //just update the rowPointers

        } else if(matARowBeginIndex == -1){ // matA row is complete zero row. copy matB row to matC

            matBNextRowBeginIndex = *(matBRowPointers+i+1);
            k=i+2;
            while(matBNextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matBNextRowBeginIndex = *(matBRowPointers + k++);
            }

            matBOffset = matBRowBeginIndex-1;
            matBOffsetToNextRow = matBNextRowBeginIndex - 1;
            matBColumnIndex = *(matBColumnIndexes+matBOffset);

            while(matBOffset < matBOffsetToNextRow){ // copy all non-zero elements of matrix B to matC
                *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                *matCNNZ += 1;
                matBOffset++;
                matBColumnIndex = *(matBColumnIndexes+matBOffset);
            }
        } else if (matBRowBeginIndex == -1){ // matB row is complete zero row. copy matA row to matC

            matANextRowBeginIndex = *(matARowPointers+i+1);
            k=i+2;
            while(matANextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matANextRowBeginIndex = *(matARowPointers + k++);
            }

            matAOffset = matARowBeginIndex-1;
            matAOffsetToNextRow = matANextRowBeginIndex - 1;
            matAColumnIndex = *(matAColumnIndexes+matAOffset);

            while(matAOffset<matAOffsetToNextRow){ // copy all remaining non-zero elements of matrix A to matC
                *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                *matCNNZ += 1;
                matAOffset++;
                matAColumnIndex = *(matAColumnIndexes+matAOffset);
            }
        } else{ // both rows are non zero

            matANextRowBeginIndex = *(matARowPointers+i+1);
            k=i+2;
            while(matANextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matANextRowBeginIndex = *(matARowPointers + k++);
            }

            matBNextRowBeginIndex = *(matBRowPointers+i+1);
            k=i+2;
            while(matBNextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matBNextRowBeginIndex = *(matBRowPointers + k++);
            }

            // handle null rows

            matAOffset = matARowBeginIndex-1;
            matBOffset = matBRowBeginIndex-1;
            matAOffsetToNextRow = matANextRowBeginIndex - 1;
            matBOffsetToNextRow = matBNextRowBeginIndex - 1;
            matAColumnIndex = *(matAColumnIndexes+matAOffset);
            matBColumnIndex = *(matBColumnIndexes+matBOffset);

            while (matAOffset<matAOffsetToNextRow || matBOffset < matBOffsetToNextRow){

                if(matAOffset<matAOffsetToNextRow && matBOffset < matBOffsetToNextRow){ // non zero elements remaining in both matrix row i
                    if(matAColumnIndex < matBColumnIndex){ // just copy matA Element at (i,matAColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                    } else if(matAColumnIndex > matBColumnIndex){  // just copy matB Element at (i,matBColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                        *matCNNZ += 1;
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    } else{ // Add matA Element at (i,matAColumnIndex) and matB Element at (i,matBColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset) + *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    }
                } else if(matAOffset<matAOffsetToNextRow){  // non zero elements remaining only in matrix A row i
                    while(matAOffset<matAOffsetToNextRow){ // copy all remaining non-zero elements of matrix A to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                    }
                } else{ // non zero elements remaining only in matrix B row i
                    while(matBOffset < matBOffsetToNextRow){ // copy all remaining non-zero elements of matrix B to matC
                        *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                        *matCNNZ += 1;
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    }
                }
            }
        }


        // updating rowPointers
        if(priorMatCNNZ<*matCNNZ){ // not a complete zero row
            *(matCRowPointers+i) = priorMatCNNZ+1;
        } else{  // complete zero row
            *(matCRowPointers+i) = -1;
        }

    }

    // readjust the memory allocation for matC associated arrays
    //matCValues = realloc(matCValues, *matCNNZ);
    //matCColumnIndexes = realloc(matCColumnIndexes, *matCNNZ);

}

void denseMatrixAddition(int *matA, int *matB, int *matC, int size){
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            *((matC+i*size)+j) = *((matA+i*size)+j) + *((matB+i*size)+j);
        }
    }
}

float elapsed_time_msec(struct timespec *begin, struct timespec *end, long *sec, long *nsec) {
    if (end->tv_nsec < begin->tv_nsec) {
        *nsec = 1000000000 - (begin->tv_nsec - end->tv_nsec);
        *sec = end->tv_sec - begin->tv_sec -1;
    }
    else {
        *nsec = end->tv_nsec - begin->tv_nsec;
        *sec = end->tv_sec - begin->tv_sec;
    }
    return (float) (*sec) * 1000 + ((float) (*nsec)) / 1000000;
}

int main() {

    const int N = 5;
    int i,j;

    //Q1 - k
    int matA[5][5] = {
            {1, 0, 0, 2, 0},
            {0, 0, 0, 4, 0},
            {0, 0, 8, 0, 0},
            {0, 8, 0, 0, 16},
            {0, 32, 0, 0, 64}
    };
    int matB[5][5] = {
            {0, 1, 0, 0, 2},
            {0, 0, 0, 4, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 16},
            {32, 0, 0, 0, 64}
    };

    int *matC;

    //Q1 - k
    /*
     * values
     * 1 1 2 2 8 8 8 32 32 32 128
     *
     * columnIndexes
     * 1 2 4 5 4 3 2 5 1 2 5
     *
     * rowPointers
     * 1 5 6 7 9 12
     *
     * */

    //Q1 - k
    int *matAValues, *matAColumnIndexes, *matARowPointers, matANNZ, *matBValues, *matBColumnIndexes,
            *matBRowPointers, matBNNZ, *matCValues, *matCColumnIndexes, *matCRowPointers, matCNNZ;

   /* *//*Timing Variables*//*
    struct timespec t0, t1, t2;
    unsigned long sec, nsec;
    float comp_time; // in milli seconds

    *//*Elapsed Time Calculation Code*//*
    GET_TIME(t0);
    // do initializations, setting-up etc
    GET_TIME(t1);
    // do computation
    GET_TIME(t2);
    comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);*/

    //Q1 - k
    matANNZ = 7;
    matBNNZ = 7;
    matCNNZ = 0;
    matAValues = (int*) malloc(matANNZ*sizeof(int));
    matAColumnIndexes = (int*) malloc(matANNZ*sizeof(int));
    matARowPointers = (int*) malloc(N*sizeof(int));
    matBValues = (int*) malloc(matBNNZ*sizeof(int));
    matBColumnIndexes = (int*) malloc(matBNNZ*sizeof(int));
    matBRowPointers = (int*) malloc(N*sizeof(int));
    matCValues = (int*) malloc((matANNZ + matBNNZ)*sizeof(int));
    matCColumnIndexes = (int*) malloc((matANNZ + matBNNZ)*sizeof(int));
    matCRowPointers = (int*) malloc(N*sizeof(int));
    matC = (int*) malloc(N*N*sizeof(int)); // to store the resulting matrix from dense algorithm

    //Q1 - k
    printf("\n*************************  Original Sparse Matrix-A **************************\n");
    print2DArray(&matA, N);
    printf("\n*****************************************************************************\n");
    printf("\n********************** Converting Mat-A To CRS Format  **********************\n");
    convertToCRS(&matA, matAValues, matAColumnIndexes, matARowPointers, N);
    printf("\nValues Array\n");
    print1DArray(matAValues, matANNZ);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(matAColumnIndexes, matANNZ);
    printf("\n\nRow Pointers\n");
    print1DArray(matARowPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    printf("\n*************************  Original Sparse Matrix-B *************************\n");
    print2DArray(&matB, N);
    printf("\n*****************************************************************************\n");
    printf("\n*********************** Converting Mat-B To CRS Format  *********************\n");
    convertToCRS(&matB, matBValues, matBColumnIndexes, matBRowPointers, N);
    printf("\nValues Array\n");
    print1DArray(matBValues, matBNNZ);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(matBColumnIndexes, matBNNZ);
    printf("\n\nRow Pointers\n");
    print1DArray(matBRowPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    printf("\n*********************** Running Dense Matrix Addition  **********************\n");
    denseMatrixAddition(&matA, &matB, matC, N);
    printf("\nResulting Matrix-C\n");
    print2DArray(matC, N);
    printf("\n\n*****************************************************************************\n");

    printf("\n*********************** Running Sparse Matrix Addition  **********************\n");
    sparseMatrixAddition(matAValues, matAColumnIndexes, matARowPointers, matANNZ,
                         matBValues, matBColumnIndexes, matBRowPointers, matBNNZ,
                         matCValues, matCColumnIndexes, matCRowPointers, &matCNNZ, N);
    printf("\nResulting CRS Format\n");
    printf("\nValues Array\n");
    print1DArray(matCValues, matCNNZ);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(matCColumnIndexes, matCNNZ);
    printf("\n\nRow Pointers\n");
    print1DArray(matCRowPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    return 0;
}