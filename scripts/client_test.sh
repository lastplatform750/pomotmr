#!/bin/bash


while true
do
if [ -S /tmp/pomotmr.sock ]; then
    remaining_time=$(socat - UNIX-CONNECT:/tmp/pomotmr.sock)
    printf "Remaining Time: %i:%02d\n" $((remaining_time / 60)) $((remaining_time % 60)) 
else
    printf "Couldn't find socket\n"
fi
done