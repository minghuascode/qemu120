#!/bin/bash

arg=$1

if [ "x$arg" == "xclean" ]; then 
  rm *.o a.out
  echo "Cleaned *.o a.out"
else

  gcc -c -I ../lib -o base64.o ../lib/base64_enc.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  gcc -c -I ../lib -o sha1.o   ../lib/sha1.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  gcc -c -I ../lib -o webs.o   ../lib/websocket.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  gcc -c -I ../lib -o main.o   main.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  gcc main.o base64.o sha1.o webs.o
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  echo "Built *.o a.out"

fi

