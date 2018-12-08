#!/bin/bash
SIZE=7000
./make_matrix -n $SIZE -l 0 -u 10 -o matrix
sleep 1
./make_matrix -n $SIZE -l 0 -u 10 -o vector -v
