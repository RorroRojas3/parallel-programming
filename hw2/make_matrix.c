#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXLENGTH 256

int main(int argc, char *argv[])
{
    // Variable Declaration Section
    FILE *file;
    double random_num = 0;
    int i, j;
    int is_vector = 0;
    int matrix_size;
    int lower_number, upper_number;
    char user_input[MAXLENGTH];
    char output_name[MAXLENGTH];

    /* DISPLAYS HOW TO RUN PROGRAM */
    if ((argc != 9) && (argc != 10))
    {
        printf("Square Matrix Usage: ./executable -n matrix_size -l low_num -u high_num -o output_binary_file\n");
        printf("Vector Usage: ./executable -n matrix_size -l low_num -u high_num -o output_binary_file -v\n");
        exit(1);
    }

    /* OBTAINS DATA FROM COMMAND LINE */
    for (i = 1; i < argc; i++)
    {
        strcpy(user_input, argv[i]);
        if (strcmp(user_input, "-n") == 0)
        {
            matrix_size = atoi(argv[i + 1]);
        }
        if (strcmp(user_input, "-l") == 0)
        {
            lower_number = atoi(argv[i + 1]);
        }
        if (strcmp(user_input, "-u") == 0)
        {
            upper_number = atoi(argv[i + 1]);
        }
        if (strcmp(user_input, "-o") == 0)
        {
            strcpy(output_name, argv[i + 1]);
        }
        if (strcmp(user_input, "-v") == 0)
        {
            is_vector = 1;
        }
    }

    file = fopen(output_name, "wb");
    if (file == NULL)
    {
        printf("Error, could not create output binary file\n");
        exit(1);
    }
    
    // Seed for random generator
    srand(time(0));

    /*  SQUARE MATRIX */
    if (argc == 9)
    {
        fprintf(file, "%d %d\n", matrix_size, matrix_size);
        for (i = 0; i < matrix_size; i++)
        {
            for (j = 0; j < matrix_size; j++)
            {
                random_num = (double)(rand() % (upper_number - lower_number + 1) + lower_number);
                fprintf(file, "%lf ", random_num);
            }
            fprintf(file, "\n");
        }
    }

    /* VECTOR */
    if ((argc == 10) && (is_vector == 1))
    {
        fprintf(file, "%d %d\n", matrix_size, 1);
        for(i = 0; i < matrix_size; i++)
        {
            random_num = (double)(rand() % (upper_number - lower_number + 1) + lower_number);
            fprintf(file, "%lf\n", random_num);
        }
    }

    fclose(file);
    return 0 ;
}