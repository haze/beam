#!/bin/sh
# bm.sh
# written by: haze

function bm() {
  OUTPUT=`beam $@`
  if [ $? -eq 2 ]
    then cd "$OUTPUT"
    else echo "$OUTPUT"
  fi
}
