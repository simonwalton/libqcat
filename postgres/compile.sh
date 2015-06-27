#!/bin/bash

rm *.o *.so

# os x
cc -I/usr/local/Cellar/postgresql/9.2.4/include/server/ -fpic -c log2.c 
gcc -bundle -flat_namespace -undefined suppress -o log2.so log2.o

# linux
#cc -I/usr/include/postgresql/9.1/server/ -fpic -c log2.c
#gcc -shared -o log2.so log2.o
