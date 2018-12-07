FILE1="make_matrix.c"
FILE2="print_matrix.c"
FILE3="lab3-parallel.c"
FILE4="lab3-serial.c"
OUT1="make_matrix"
OUT2="print_matrix"
OUT3="parallel"
OUT4="serial"
gcc -Wall -g $FILE1 -o $OUT1
gcc -Wall -g $FILE2 -o $OUT2
gcc -Wall -g $FILE3 -o $OUT3 -fopenmp
gcc -Wall -g $FILE4 -o $OUT4
