#!/bin/bash
# scripts/cfg1.sh

blddir=`pwd`

sh ../src/configure --prefix=${blddir}/../bin  \
        --target-list="arm-softmmu arm-linux-user armeb-linux-user"  \
        --source-path=${blddir}/../src

exit 0

#

