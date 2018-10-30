#include <stdio.h>
#include <stdlib.h>

#define MAXLENGTH 1048576

int main(int argc, char *argv[])
{
    // Variable Declaration Section
    FILE *file;
    char line[MAXLENGTH];
    int first_line = 0;

    /*  DISPLAYS HOW TO RUN PROGRAM */
    if (argc != 2)
    {
        printf("Usage: ./executable input_file\n");
        exit(1);
    }

    /* READS BINARY FILE */
    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf("Error, could not open input file\n");
        exit(1);
    }

    /* PRINTS MATRIX/VECTOR TO THE CONSOLE */
    while(fgets(line, sizeof(line), file) != NULL)
    {
        // Allows to skip first line of matrix
        if (first_line == 1)
        {
            printf("%s", line);
        }
        else
        {
            first_line = 1;
        }
    }

    /* CLOSES OPENED FILE */
    fclose(file);

    return 0;
}