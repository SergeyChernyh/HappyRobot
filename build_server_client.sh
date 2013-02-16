CXX=$*

OPTS="-pthread -Wall -Werror -std=c++11 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 "
INCLUDE="-Ir_lib"

killall server

$CXX $OPTS $INCLUDE ./server.cpp; mv ./a.out server
$CXX $OPTS $INCLUDE ./client.cpp; mv ./a.out client
