FILE1="make_matrix.c"
FILE2="print_matrix.c"
FILE3="multiply_matrix.c"
OUT1="make_matrix"
OUT2="print_matrix"
OUT3="multiply_matrix"
gcc -Wall -g $FILE1 -o $OUT1
gcc -Wall -g $FILE2 -o $OUT2
gcc -Wall -g $FILE3 -o $OUT3
