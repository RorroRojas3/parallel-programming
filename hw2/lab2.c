/*
	Name: Rodrigo Ignacio Rojas Garcia
	HW#2
*/

// LIBRARY DECLARATION SECTION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// DEFINE DECLARATION SECTION
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n) -1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n)-BLOCK_LOW(id, p, n))
#define BLOCK_OWNER(index, p, n) (((p)*((index)+1)-1)/(n))
#define MAXLENGTH 256

int main(int argc, char **argv)
{
    // VARIABLE DECLARATION SECTION
    int i, j, k;
	int offset; 
	int num_of_items;  /* total data items in each dim */
	int SIZE, RANK; /* original rank and size */
	int cartesian_rank; /* local rank in the 2D topology */
	int coords[2]; /* local coordinates in the 2D topology */
	int row_start, row_end, row_count;
	int col_start, col_end, col_count;
	int matrix_row, matrix_col, vector_row, vector_col;
	int size;
	double *matrix, *vector;
	double *b_vector, *matrix_A;
	double *div_matrix_A, *div_vector_B;
	double *multiplication_result, result;
	double *c_vector;
	char file_name[MAXLENGTH];
	FILE *matrix_file, *vector_file, *output_file;
	matrix_row = matrix_col = vector_row = vector_col = 0;
	offset = 0; 

	// SIZE OF TOPOLOGY IN EACH DIRECTION
	int grid_size[2] = {0, 0};  
	
	// CREATE COMMUNICATORS 
	MPI_Comm grid_comm, row_comm, column_comm;

	// ARRAY WITH INFORMATION REGARDING TOPOLOGY (NO WRAP AROUND)
	int period[2] = {0, 0}; 

	// MPI INITIALIZATION 
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);

	// DETERMINES TOPOLOGY SIZE
	MPI_Dims_create(SIZE, 2, grid_size);
 
 	// CREATE 2D TOPOLOGY
	MPI_Cart_create(MPI_COMM_WORLD, 2, grid_size, period, 0, &grid_comm);

    // GETS THE RANK FROM CARTESIAN COMMUNICATOR AND COORDINATES 
	MPI_Comm_rank(grid_comm, &cartesian_rank);           /* get 2D rank … */
	MPI_Cart_coords(grid_comm, cartesian_rank, 2, coords);  /* then 2D coords */

    // DIVIDES CARTESIAN COMMUNICATOR INTO ROW AND COLUMN COMMUNICATOR 
	MPI_Comm_split(MPI_COMM_WORLD, coords[0], coords[1], &row_comm); 
	MPI_Comm_split(MPI_COMM_WORLD, coords[1], coords[0], &column_comm); 

	// MAKES ALL PROCESSESES WAIT UNTIL REACH BARRIER
	MPI_Barrier(MPI_COMM_WORLD);
	
	// READ IN THE VECTORS BY CARTESIAN PROCESS 0
	if (cartesian_rank  == 0)
	{
		// OBTAINS INFORMATION FROM COMMAND LINE TO OPEN/CREATE FILES 
		matrix_file = fopen("matrix", "rb");
		if (matrix_file == NULL)
		{
			printf("Error, could not read first input file\n");
			exit(1);
		}
		vector_file = fopen("vector", "rb");
		if (vector_file == NULL)
		{
			printf("Error, could not read second input file\n");
			fclose(matrix_file);
			exit(1);
		}

		// GETS SIZE OF SQUARE MATRIX 
		fscanf(matrix_file, "%d %d", &matrix_row, &matrix_col);
		fscanf(vector_file, "%d %d", &vector_row, &vector_col);

		// NOTE: ALWAYS GONNA WORK
		if (matrix_col != vector_row)
		{
			printf("Error, Matrix/Vector must have same COLUMN-ROW value\n");
			fclose(matrix_file);
			fclose(vector_file);
			exit(1);
		}
		else
		{
			printf("New Matrix size: %dx%d\n", matrix_row, vector_col);
		}

		  //ALLOCATE MEMORY FOR MATRICES 
    	matrix = (double *)calloc(matrix_row * matrix_row, sizeof(double));
   		vector = (double *)calloc(vector_row, sizeof(double));

		// EXTRACT DATA FROM INPUT MATRIX FILES AND STORE THEM ON ALLOCATE VARIABLES 
		for (i = 0; i < (matrix_row * matrix_col); i++)
		{
			fscanf(matrix_file, "%lf", &matrix[i]);
		}
		for (i = 0; i < vector_row; i++)
		{
			fscanf(vector_file, "%lf", &vector[i]);
		}
		
		num_of_items = matrix_row;
        
		// SEND ARRAY SIZE TO ALL CARTESIAN PROCESSES
		for (i = 0; i < SIZE; i++)
		{
			MPI_Send(&num_of_items, 1, MPI_INT, i, 1, grid_comm);
		}

		// SENDS "B" VECTOR AND "A" MATRIX TO CARTESIAN RANK 0
		MPI_Send(vector, num_of_items, MPI_DOUBLE, 0, 1, grid_comm);
		MPI_Send(matrix, num_of_items * num_of_items, MPI_DOUBLE, 0, 1, grid_comm);

		// FREE ALLOCATED MEMORY
		free(matrix);
		free(vector);

		// CLOSE OPENED FILES 
		fclose(matrix_file);
		fclose(vector_file);
	}

	// RECEIVES SIZE OF ARRAY
	MPI_Recv(&num_of_items, 1, MPI_INT, 0, 1, grid_comm, MPI_STATUS_IGNORE);
	
	// ALLOCATE DYNAMIC MEMORY
	size = (num_of_items / grid_size[1]);
	div_vector_B = (double *)calloc(size, sizeof(double));
	div_matrix_A = (double *)calloc(size * size, sizeof(double));
	b_vector = (double *)calloc(num_of_items, sizeof(double));
	matrix_A = (double *)calloc(num_of_items * num_of_items, sizeof(double));
	multiplication_result = (double *)calloc(size, sizeof(double));
	c_vector = (double *)calloc(size, sizeof(double));

	if (cartesian_rank == 0)
	{
		MPI_Recv(b_vector, num_of_items, MPI_DOUBLE, 0, 1, grid_comm, MPI_STATUS_IGNORE);
		MPI_Recv(matrix_A, (num_of_items * num_of_items), MPI_DOUBLE, 0, 1, grid_comm, MPI_STATUS_IGNORE);
	}
		
	MPI_Barrier(grid_comm);

	// SCATTER MATRIX A AMONG ALL PROCESSES
	MPI_Scatter(matrix_A, size * size, MPI_DOUBLE, div_matrix_A, size * size, MPI_DOUBLE, 0, grid_comm);

	MPI_Barrier(grid_comm);
	free(matrix_A);

	MPI_Barrier(grid_comm);

	// SCATTER "B" VECTOR ACROSS COLUMN COMMUNICATOR
	if (coords[1] == 0)
	{
		MPI_Scatter(b_vector, size, MPI_DOUBLE, div_vector_B, size, MPI_DOUBLE, 0, column_comm);
		
		MPI_Bcast(div_vector_B, size, MPI_DOUBLE, 0, row_comm);
	}	
	MPI_Barrier(grid_comm);
	MPI_Bcast(div_vector_B, size, MPI_DOUBLE, 0, row_comm);

	/*row_start = BLOCK_LOW(coords[0],  grid_size[0], num_of_items);
	row_end   = BLOCK_HIGH(coords[0], grid_size[0], num_of_items);
	row_cnt   = BLOCK_SIZE(coords[0], grid_size[0], num_of_items); 

	col_start = BLOCK_LOW(coords[1],  grid_size[1], num_of_items);
	col_end   = BLOCK_HIGH(coords[1], grid_size[1], num_of_items); 
	col_cnt   = BLOCK_SIZE(coords[1], grid_size[1], num_of_items); 
	*/

	// MATRIX MULTIPLICATION
	for (i = 0; i < size; i++)
	{
		result = 0;
		for (j = 0; j < size; j++)
		{
			k = (i * 4) + j;
			result += (div_matrix_A[k] * div_vector_B[j]);
		}
		multiplication_result[i] = result;
	}

	// SUMS VALUES ACROSS ROW PROCESSES AND RESULT RETURNED TO C_VECTOR
	MPI_Barrier(grid_comm);
	MPI_Reduce(multiplication_result, c_vector, size, MPI_DOUBLE, MPI_SUM, 0, row_comm);

	// CREATES FILE TO OUTPUT "C" VECTOR
	if (coords[1] == 0)
	{
		memset(file_name, '\0', sizeof(file_name));
		sprintf(file_name, "C-vector-%d", cartesian_rank);
		output_file = fopen(file_name, "wb");
		for (i = 0; i < size; i++)
		{
			fprintf(output_file, "%lf\n", c_vector[i]);
		}
	}
	
  
	MPI_Finalize();

	return 0;
}
