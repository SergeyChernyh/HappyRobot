#include "p2_at.h"
#include "tcp.h"
#include "motion_control.h"
#include "common_protocol_parser.h"

using namespace robot;

int main(int argc, const char* argv[])
{
    using namespace subsystem;

    tcp_test::TCPInterface tcp;

    tcp.client_connect();

    p2_at::p2_at_device<tcp_test::TCPInterface> p2_at_mephi(tcp);

    auto& p2_at_mephi_subsystems = p2_at_mephi.get_subsystems();
    auto& move_ctrl = p2_at_mephi_subsystems.move_ctrl;

    std::thread t([&p2_at_mephi](){ p2_at_mephi.start(); });

    at_key<set_angular_vel>(move_ctrl).set(0);
    at_key<set_linear_vel >(move_ctrl).set(1200);
    at_key<linear_move    >(move_ctrl).set(1);

    package_creation::package<package_creation::pattern<uint8_t, uint8_t, int64_t, uint8_t, bool>> test_pack(uint8_t(2), uint8_t(0xe), int64_t(-1200), uint8_t(0x18), true);
    package_creation::package<package_creation::pattern<uint8_t, uint8_t, int64_t, int64_t>> test_pack_0(uint8_t(1), uint8_t(0x14), int64_t(-1200), uint64_t(10000));

    common_protocol::function_map functions;

    functions.add_function(common_protocol::function(move_ctrl), 0x1);

    for(auto& sonar : p2_at_mephi_subsystems.sonar_bar)
        functions.add_function(sonar, 0x2);

    functions[0x1][0].change_parameters(test_pack.get_data());
    functions[0x1][0].change_parameters(test_pack_0.get_data());

    tcp_test::TCPInterface tcp_external(5200);
    common_protocol::server<tcp_test::TCPInterface> server(functions, tcp_external);

    tcp_external.server_start();

    while(1)
        server.parse();

    t.join();

    return 0;
}
