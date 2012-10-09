#!/bin/bash
# scripts/cfg2.sh

blddir=`pwd`

sh ../src/configure --prefix=${blddir}/../bin  \
        --target-list="arm-softmmu"  \
        --source-path=${blddir}/../src

exit 0

