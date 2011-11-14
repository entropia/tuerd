#!/bin/bash
gcc -I../libmf/include/ -pthread -I/usr/include/PCSC -lpcsclite $1 -L../libmf/build/src -lmf -g -std=c99 -lzmq -lgcrypt
