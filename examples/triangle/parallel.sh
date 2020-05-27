#!/bin/bash

for ((i=0; i < $2;i++)); do {
  echo "Process \"$1\" started";
  $1 & pid=$!
  PID_LIST+=" $pid";
} done

trap "kill $PID_LIST" SIGINT

echo "Parallel processes have started";

wait $PID_LIST

echo
echo "All processes have completed";
