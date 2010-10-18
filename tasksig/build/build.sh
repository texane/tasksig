#!/usr/bin/env sh

gcc -Wall -O3 -I$HOME/install/include ../src/main.c -L$HOME/install/lib -lsparse