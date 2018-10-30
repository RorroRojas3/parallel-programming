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
  // VARIABLE DECLARATION SECTION
	int i, j;
  int buffer;
  int send_buffer;
	char name[10]; 
	int offset; 
	//int root; 
	int n;  /* total data items in each dim */
	//int *rtmp, *ctmp; 
	MPI_Status stat; 
	offset = 0; 
	int SIZE, RANK; /* original rank and size */
	int cartesian_rank; /* local rank in the 2D topology */
	int coords[2]; /* local coordinates in the 2D topology */
	int row_start;
	int row_end;
	int row_cnt;
	int col_start;
	int col_end;
	int col_cnt;

	/* size of topology grid in each direction */
	int grid_size[2];  
	
	/* declare several communicators */
	MPI_Comm grid_comm, Comm_row, Comm_col;

	/* arrays with info about the 2D topology */
	int period[2] = {0, 0}; /* do not wrap around cart dims */

	// MPI INITIALIZATION 
	MPI_Init(&argc, &argv);

	/* must read n from file or user */

	MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);

	/* get cart topology size */
	/*	NUMBER OF NODES IN GRID, CARTESIAN DIMENSIONS, NODES IN EACH DIMENSION */
	// DIVIDES EVERYTHING EVENLY AMONG TASKS 
	MPI_Dims_create(SIZE, 2, grid_size);
 
 	/* create 2D cart topology */
	/*	OLD COMMUNICATOR, TWO DIMENSIONAL, SIZE OF MATRIX (R,C), NO-WRAP
		NO-REORDER, NEW COMMUNICATOR */
	MPI_Cart_create(MPI_COMM_WORLD, 2, grid_size, period, 0, &grid_comm);

  // GETS THE RANK FROM CARTESIAN COMMUNICATOR AND COORDINATES 
	MPI_Comm_rank(grid_comm, &cartesian_rank);           /* get 2D rank … */
	MPI_Cart_coords(grid_comm, cartesian_rank, 2, coords);  /* then 2D coords */

  // DIVIDES CARTESIAN COMMUNICATOR INTO ROW AND COLUMN COMMUNICATOR 
	MPI_Comm_split(MPI_COMM_WORLD, coords[0], coords[1], &Comm_row); 
	MPI_Comm_split(MPI_COMM_WORLD, coords[1], coords[0], &Comm_col); 

	// MAKES ALL PROCESSESES WAIT UNTIL REACH BARRIER
	MPI_Barrier(MPI_COMM_WORLD);
	
	// READ IN THE VECTORS BY CARTESIAN PROCESS 0
	if (cartesian_rank  == 0)
	{
		// VARIABLE DECLARATION SECTION
		FILE *matrix_file, *vector_file;
		int i, matrix_row, matrix_col, vector_row, vector_col;
    int dest_coords[2];
    int dest_id;
    int grid_id;
    int grid_coords[2];
		double **matrix, *vector;
		matrix_row = matrix_col = vector_row = vector_col = 0;
   
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
    	matrix = (double **)calloc(matrix_row, sizeof(double *));
   		vector = (double *)calloc(vector_row, sizeof(double));

    for (i = 0 ; i < matrix_row; i++)
		{
			matrix[i] = (double *)calloc(matrix_col, sizeof(double));
		}

		// EXTRACT DATA FROM INPUT MATRIX FILES AND STORE THEM ON ALLOCATE VARIABLES 
		for (i = 0; i < matrix_row; i++)
		{
			for (j = 0; j < matrix_col; j++)
			{
				fscanf(matrix_file, "%lf", &matrix[i][j]);
			}
		}
		for (i = 0; i < vector_row; i++)
		{
			for (j = 0; j < vector_col; j++)
			{
				fscanf(vector_file, "%lf", &vector[i]);
			}
		}

		n = matrix_row;		

		/* find row start and end index, then same for column */ 
		row_start = BLOCK_LOW(coords[0],  grid_size[0], n);
		row_end   = BLOCK_HIGH(coords[0], grid_size[0], n);
		row_cnt   = BLOCK_SIZE(coords[0], grid_size[0], n); 

		col_start = BLOCK_LOW(coords[1],  grid_size[1], n);
		col_end   = BLOCK_HIGH(coords[1], grid_size[1], n); 
		col_cnt   = BLOCK_SIZE(coords[1], grid_size[1], n);
    
    printf("Rank: %d, Row Start: %d, Row End: %d, Row Count: %d\n", 
            cartesian_rank, row_start, row_end, row_cnt);
    printf("Rank: %d, Col Start: %d, Col End: %d, Col Count: %d\n", 
            cartesian_rank, col_start, col_end, col_cnt);
    
    MPI_Cart_coords(grid_comm, cartesian_rank, 2, grid_coords);
    printf("Rank: %d, Coords: %d %d\n\n", cartesian_rank, grid_coords[0], grid_coords[1]); 
    
    // SENDS MATRIX SIZE TO OTHER CARTERSIAN PROCESSES 
    MPI_Bcast(&n, 1, MPI_INT, cartesian_rank, grid_comm);
    
    for (i = 0; i < n; i++)
    {
      dest_coords[0] = BLOCK_OWNER(i, grid_size[0], n);
      dest_coords[1] = 0;
      MPI_Cart_rank(grid_comm, dest_coords, &dest_id);
      if (cartesian_rank == 0)
      {
        double *row;
        row = (double *)calloc(n, sizeof(double));
        for (j = 0; j <  n; j++)
        {
          row[j] = matrix[i][j];
        }
        MPI_Send(&row, n, 
        
      }
      
    }
	}
 
	// OTHER CARTESIAN PROCESSES 
  else
  {
    // RECEIVES SIZE OF MATRIX FROM CARTESIAN RANK 0
    MPI_Bcast(&buffer, 1, MPI_INT, 0, grid_comm );
    n = buffer;
    

    /* find row start and end index, then same for column */ 
		row_start = BLOCK_LOW(coords[0],  grid_size[0], n);
		row_end   = BLOCK_HIGH(coords[0], grid_size[0], n);
		row_cnt   = BLOCK_SIZE(coords[0], grid_size[0], n); 
    col_start = BLOCK_LOW(coords[1],  grid_size[1], n);
		col_end   = BLOCK_HIGH(coords[1], grid_size[1], n); 
		col_cnt   = BLOCK_SIZE(coords[1], grid_size[1], n);
    printf("Rank: %d, Row Start: %d, Row End: %d, Row Count: %d\n", 
            cartesian_rank, row_start, row_end, row_cnt);
    printf("Rank: %d, Col Start: %d, Col End: %d, Col Count: %d\n", 
            cartesian_rank, col_start, col_end, col_cnt);
		


    int grid_coords[2];
    MPI_Cart_coords(grid_comm, cartesian_rank, 2, grid_coords);
    printf("Rank: %d, Coords: %d %d\n", cartesian_rank, grid_coords[0], grid_coords[1]); 
  }  

	
  
	MPI_Finalize();
}
