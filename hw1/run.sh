#!/bin/bash
FILENAME="smallball.c"
OUTPUT="smallball"
mpicc -g -Wall -Wstrict-prototypes $FILENAME -o $OUTPUT
mpiexec -n 4 ./$OUTPUT
