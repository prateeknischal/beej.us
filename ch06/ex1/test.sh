#!/bin/bash

function make_connection() {
    random_string=$(head -c16 /dev/urandom | base64)
    echo -e $random_string | nc localhost 9000
}

export -f make_connection
seq 1 1000 | parallel -j 32 make_connection
