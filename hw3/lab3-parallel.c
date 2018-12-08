#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define MAXLENGTH 256

int main(int argc, char *argv[])
{
    // Variable Declaration Section
    FILE *matrix_file1, *matrix_file2, *output_matrix_file;
    int i, j, k, matrix_row1, matrix_col1, matrix_row2, matrix_col2;
    double **matrix1, **matrix2, **output_matrix;
    matrix_row1 = matrix_col1 = matrix_row2 = matrix_col2 = 0;
    clock_t start_time, end_time;
    double cpu_time_used = 0;

    /*  DISPLAYS HOW TO RUN PROGRAM */
    
    if (argc != 4)
    {
        printf("Usage: ./executable matrix1_file matrix2_file output_matrix_file\n");
        exit(1);
    }

    /* OBTAINS INFORMATION FROM COMMAND LINE TO OPEN/CREATE FILES */
    matrix_file1 = fopen(argv[1], "rb");
    if (matrix_file1 == NULL)
    {
        printf("Error, could not read first input file\n");
        exit(1);
    }
    matrix_file2 = fopen(argv[2], "rb");
    if (matrix_file2 == NULL)
    {
        printf("Error, could not read second input file\n");
        fclose(matrix_file1);
        exit(1);
    }
    output_matrix_file = fopen(argv[3], "wb");
    if (output_matrix_file == NULL)
    {
        printf("Error, could not create output file\n");
        fclose(matrix_file1);
        fclose(matrix_file2);
        exit(1);
    }

    /* GETS SIZE OF SQUARE MATRIX */
    fscanf(matrix_file1, "%d %d", &matrix_row1, &matrix_col1);
    fscanf(matrix_file2, "%d %d", &matrix_row2, &matrix_col2);

    if (matrix_col1 != matrix_row2)
    {
        printf("Error, Matrix/Vector must have same COLUMN-ROW value\n");
        fclose(matrix_file1);
        fclose(matrix_file2);
        fclose(output_matrix_file);
        exit(1);
    }
    else
    {
        printf("New Matrix size: %dx%d\n", matrix_row1, matrix_col2);
    }

    /* ALLOCATE MEMORY FOR MATRICES */
    matrix1 = (double **)calloc(matrix_row1, sizeof(double *));
    matrix2 = (double **)calloc(matrix_row2, sizeof(double *));
    output_matrix = (double **)calloc(matrix_row1, sizeof(double *));
    for (i = 0 ; i < matrix_row1; i++)
    {
        matrix1[i] = (double *)calloc(matrix_col1, sizeof(double));
        output_matrix[i] = (double *)calloc(matrix_col2, sizeof(double));
    }
    for (i = 0 ; i < matrix_row2; i++)
    {
        matrix2[i] = (double *)calloc(matrix_col2, sizeof(double));
    }

    /* EXTRACT DATA FROM INPUT MATRIX FILES AND STORE THEM ON ALLOCATE VARIABLES */
    for (i = 0; i < matrix_row1; i++)
    {
        for (j = 0; j < matrix_col1; j++)
        {
            fscanf(matrix_file1, "%lf", &matrix1[i][j]);
        }
    }
    for (i = 0; i < matrix_row2; i++)
    {
        for (j = 0; j < matrix_col2; j++)
        {
            fscanf(matrix_file2, "%lf", &matrix2[i][j]);
        }
    }

    start_time = clock();
    /* CALCULATE OUTPUT MATRIX */
	omp_set_num_threads(8);
	#pragma omp parallel for private(j, k)
    for (i = 0; i < matrix_row1; i++)
    {
        for (j = 0; j < matrix_col2; j++)
        {
            for (k = 0; k < matrix_row2; k++)
            {
                output_matrix[i][j] += (matrix1[i][k] * matrix2[k][j]);
            }   
        }
    }
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Parallel time: %.20f\n", cpu_time_used);

    /* STORE MULTIPLICATION IN BINARY FILE  */
    fprintf(output_matrix_file, "%d %d\n",   matrix_row1, matrix_col2);
    for (i = 0; i < matrix_row1; i++)
    {
        for (j = 0; j < matrix_col2; j++)
        {
            fprintf(output_matrix_file, "%lf ", output_matrix[i][j]);
        }
        fprintf(output_matrix_file, "\n");
    }

    /* FREE ALLOCATED MEMORY */
    for (i = 0; i < matrix_row1; i++)
    {
        free(matrix1[i]);
        free(output_matrix[i]);
    }
    for (i = 0; i < matrix_row2; i++)
    {
        free(matrix2[i]);   
    }
    free(matrix1);
    free(matrix2);
    free(output_matrix);


    /* CLOSES OPENED/CREATED FILES */
    fclose(matrix_file1);
    fclose(matrix_file2);
    fclose(output_matrix_file);
    
    return 0;
}
