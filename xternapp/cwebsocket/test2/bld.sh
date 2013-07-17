#!/bin/bash

arg=$1

#uncomment the line below for arm cross build
#pre=arm-none-linux-gnueabi-

lib=../lib
src=../x86_server

if [ ! -d bld.dir ]; then 
  mkdir bld.dir
fi

cd bld.dir
echo -n "pwd: "
pwd

if [ "x$arg" == "xclean" ]; then 
  rm *.o a.out
  echo "Cleaned *.o a.out"
else

  ${pre}gcc -g -c -I ${lib} -o base64.o ${lib}/base64_enc.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  ${pre}gcc -g -c -I ${lib} -o sha1.o   ${lib}/sha1.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  ${pre}gcc -g -c -I ${lib} -o webs.o   ${lib}/websocket.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  ${pre}gcc -g -c -I ${lib} -o test2.o  ${src}/test2.c
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  ${pre}gcc -g test2.o base64.o sha1.o webs.o
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  cp ../wstest*.html .
  if [ ! "x$?" == "x0" ]; then echo error; exit 1; fi

  echo "Built *.o a.out"

fi

