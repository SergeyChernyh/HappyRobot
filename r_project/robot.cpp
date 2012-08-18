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

    auto& move_ctrl = p2_at_mephi.get_subsystems().move_ctrl;

    std::thread t([&p2_at_mephi](){ p2_at_mephi.start(); });

    at_key<subsystem::set_angular_vel>(move_ctrl).set(0);
    at_key<subsystem::set_linear_vel >(move_ctrl).set(1200);
    at_key<subsystem::move           >(move_ctrl).set(1);

    t.join();

    return 0;
}
