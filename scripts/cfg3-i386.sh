#!/bin/bash
# scripts/cfg2.sh

blddir=`pwd`

sh ../src/configure --prefix=${blddir}/../bin  \
        --target-list="i386-softmmu"  \
        --source-path=${blddir}/../src

exit 0

