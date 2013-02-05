#!/bin/bash

COMPILER=$1
ARGS="-pthread -Wall -Werror -std=c++11 -Ir_lib"

check() {
    rm -f ./a.out
    $COMPILER $ARGS tests/$1
    ./a.out
    if [ $? -eq 0 ]
    then
        echo "$1 passed"
    else
        echo "$1 failed"
        echo "TEST FAILED"
        exit 1
    fi
}

check data_types.cpp
check connection.cpp
check common_protocol.cpp
check device.cpp

echo "TEST PASSED"

#MobileSim --map /usr/local/MobileSim/columbia.map --robot p2at&
$COMPILER $ARGS tests/p2_at_test_new.cpp
#./a.out&
