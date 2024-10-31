#!/bin/bash
gcc -c main.c -o main.o -g
gcc -c json.c -o json.o -g
gcc main.o json.o -o a.out -g
./a.out
