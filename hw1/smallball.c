#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
	// Variable Declaration Section
	int rank;
	int size;
	int ball;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank != 0)
	{
		MPI_Recv(&ball, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d, received ball from process %d\n", rank, ball);
		MPI_Ssend(&rank, 1, MPI_INT, (rank + size + 1) % size, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Ssend(&rank, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
		MPI_Recv(&ball, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d, received ball from process %d\n", rank, ball);
	}

	MPI_Finalize();

	return 0;
}
