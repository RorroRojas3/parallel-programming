#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n) -1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n)-BLOCK_LOW(id, p, n))
#define BLOCK_OWNER(index, p, n) (((p)*((index)+1)-1)/(n))


int main(int argc, char **argv)
{
	int i, j, k; 
	char name[10]; 
	int offset; 
	int root; 
	int n;  /* total data items in each dim */
	int *rtmp, *ctmp; 
	MPI_Status stat; 
	offset = 0; 
	int SIZE, RANK; /* original rank and size */
	int rank; /* local rank in the 2D topology */
	int coords[2]; /* local coordinates in the 2D topology */
	int row_start;
	int row_end;
	int row_cnt;
	int col_start;
	int col_end;
	int col_cnt;

	/* size of topology grid in each direction */
	int gridsz[2];  
	
	/* declare several communicators */
	MPI_Comm Comm_2D, Comm_row, Comm_col;

	/* arrays with info about the 2D topology */
	int period[2] = {0, 0}; /* do not wrap around cart dims */

	/*————————————————————————————————————————————————————*/
	MPI_Init(&argc, &argv);

	/* must read n from file or user */

	MPI_Comm_size(&SIZE, MPI_COMM_WORLD);
	MPI_Comm_rank(&RANK, MPI_COMM_WORLD);

	/* get cart topology size */
	/*	NUMBER OF NODES IN GRID, CARTESIAN DIMENSIONS, NODES IN EACH DIMENSION */
	// DIVIDES EVERYTHING EVENLY AMONG TASKS 
	MPI_Dims_create(SIZE, 2, gridsz);

	/* create 2D cart topology */
	/*	OLD COMMUNICATOR, TWO DIMENSIONAL, SIZE OF MATRIX (R,C), NO-WRAP
		NO-REORDER, NEW COMMUNICATOR */
	MPI_Cart_create(MPI_COMM_WORLD, 2, gridsz, period, 0, &Comm_2D);

	MPI_Comm_rank(&rank, Comm_2D);           /* get 2D rank … */
	MPI_Cart_coords(Comm_2D, rank, 2, coords);  /* then 2D coords */

	MPI_Comm_split(MPI_COMM_WORLD, coords[0], coords[1], &Comm_row); 
	MPI_Comm_split(MPI_COMM_WORLD, coords[1], coords[0], &Comm_col);

	// READ IN THE VECTORS BY PROCESS 0 
	if (rank == 0)
	{
		FILE *matrix_file1, *matrix_file2;
		int i  matrix_row1, matrix_col1, matrix_row2, matrix_col2;
		double **matrix1, *matrix2;
		double result = 0;
		matrix_row1 = matrix_col1 = matrix_row2 = matrix_col2 = 0;

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

		/* GETS SIZE OF SQUARE MATRIX */
		fscanf(matrix_file1, "%d %d", &matrix_row1, &matrix_col1);
		fscanf(matrix_file2, "%d %d", &matrix_row2, &matrix_col2);

		// NOTES: ALWAYS GONNA WORK
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
   		matrix2 = (double *)calloc(matrix_row2, sizeof(double));

	    for (i = 0 ; i < matrix_row1; i++)
		{
			matrix1[i] = (double *)calloc(matrix_col1, sizeof(double));
		}
	}

	/* find row start and end index, then same for column */
	row_start = BLOCK_LOW(coords[0],  gridsz[0], n);
	row_end   = BLOCK_HIGH(coords[0], gridsz[0], n);
	row_cnt   = BLOCK_SIZE(coords[0], gridsz[0], n); 

	col_start = BLOCK_LOW(coords[1],  gridsz[1], n);
	col_end   = BLOCK_HIGH(coords[1], gridsz[1], n); 
	col_cnt   = BLOCK_SIZE(coords[1], gridsz[1], n); 

	MPI_Finalize();
}