#include "p2_at.h"
#include "tcp.h"

#include <iostream>
#include <array>
#include <vector>
#include <typeinfo>

using namespace robot;

int main(int argc, const char* argv[])
{
    tcp_test::TCPInterface tcp;

    tcp.open();

    p2_at::p2_at_device<tcp_test::TCPInterface> p2_at_mephi(tcp);

    return 0;
}
