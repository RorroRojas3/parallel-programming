#!/bin/bash
FILENAME="lab2.c"
OUTPUT="topology"
mpicc -g -Wstrict-prototypes $FILENAME -o $OUTPUT
mpiexec -n 8 ./$OUTPUT
