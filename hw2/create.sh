#!/bin/bash
./make_matrix -n 4096 -l 0 -u 10 -o matrix
sleep 1
./make_matrix -n 4096 -l 0 -u 10 -o vector -v
