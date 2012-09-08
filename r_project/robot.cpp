#include "p2_at.h"
#include "tcp.h"
#include "motion_control.h"
#include "common_protocol.h"
#include "common_protocol_function.h"
#include <iostream>
#include <array>
#include <vector>
#include <typeinfo>

using namespace robot;

int main(int argc, const char* argv[])
{
    using namespace subsystem;

    tcp_test::TCPInterface tcp;

    tcp.open();

    p2_at::p2_at_device<tcp_test::TCPInterface> p2_at_mephi(tcp);

    auto& p2_at_mephi_subsystems = p2_at_mephi.get_subsystems();
    auto& move_ctrl = p2_at_mephi_subsystems.move_ctrl;

    std::thread t([&p2_at_mephi](){ p2_at_mephi.start(); });

    at_key<set_angular_vel>(move_ctrl).set(0);
    at_key<set_linear_vel >(move_ctrl).set(1200);
    at_key<move           >(move_ctrl).set(1);

    using m_ctrl = motion_control<decltype(move_ctrl), p2_at::p2_at_metrics::millimetre>;
    m_ctrl ctrl(move_ctrl);

    for(auto& sonar : p2_at_mephi_subsystems.sonar_bar)
        ctrl.add_point(sonar);

    using p_t = common_protocol::parameter_rw<metaprogramming::at_key<set_linear_vel, p2_at::p2_at_subsystems::move_t>>;

    p_t p(at_key<set_linear_vel >(move_ctrl));

    uint64_t tmp = 1200ull;
    p.set_val((const uint8_t*)&tmp);

    t.join();

    return 0;
}
