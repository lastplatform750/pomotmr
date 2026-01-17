#!/bin/bash


while true
do
    if [ -S /tmp/pomotmr.sock ]; then
        sock_output=$(socat - UNIX-CONNECT:/tmp/pomotmr.sock)
        echo "Output string: $sock_output"
        p_state=${sock_output:0:1}
        remaining_time=${sock_output:1}
        printf "p_state: %c\n" $p_state
        printf "Remaining Time: %i:%02d\n" $((remaining_time / 60)) $((remaining_time % 60)) 
    else
        printf "Couldn't find socket\n"
        sleep 1
    fi
done