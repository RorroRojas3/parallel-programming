#!/bin/bash
FILENAME="lab2.c"
OUTPUT="topology"
mpicc -g -Wall -Wstrict-prototypes $FILENAME -o $OUTPUT
mpiexec -n 4 ./$OUTPUT
