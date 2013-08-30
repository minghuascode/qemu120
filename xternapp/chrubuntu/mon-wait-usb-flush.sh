#!/bin/bash


function waitforusb() {
  fincnt=0
  finstart=`date`
  while true
  do
      retv=`top -n1 -b | grep 'usb-storage'`
      retr=`echo $retv | egrep -v 'S [ ]*0'`
      rets=`echo -n $retv`
      echo ""
      echo -n "  $rets    "
      date
      if [[ "$retv" == "" || "$retr" == "" ]]; then 
        fincnt=$(($fincnt + 1))
        echo found S0 ... $fincnt
      else
        fincnt=0
      fi
      if [ $fincnt -gt 5 ]; then 
        echo found S0 ... many times finish
        break
      fi
      sleep 1
  done
  finfinish=`date`
  echo "  waitforusb started   $finstart"
  echo "  waitforusb finished  $finfinish"
}

function waitforflush() {
  fincnt=0
  finstart=`date`
  while true
  do
      echo ""
      top -n1 -b | grep 'flush' 
      echo ""
      retv=`top -n1 -b | grep 'flush' | egrep -v 'S [ ]*0'`
      rets=`echo -n $retv`
      echo ""
      echo -n "  $rets    "
      date
      if [ "$retv" == "" ]; then 
        fincnt=$(($fincnt + 1))
        echo found S0 ... $fincnt
      else
        fincnt=0
      fi
      if [ $fincnt -gt 5 ]; then 
        echo found S0 ... many times finish
        break
      fi
      sleep 1
  done
  finfinish=`date`
  echo "  waitforusb started   $finstart"
  echo "  waitforusb finished  $finfinish"
}

#waitforusb
waitforflush


