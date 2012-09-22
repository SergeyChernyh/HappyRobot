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

    common_protocol::function_map functions;

    functions.add_function(common_protocol::function(move_ctrl), 0x1);

    for(auto& sonar : p2_at_mephi_subsystems.sonar_bar)
        functions.add_function(sonar, 0x2);

    tcp_test::TCPInterface tcp_external(5200);
    common_protocol::server<tcp_test::TCPInterface> server(functions, tcp_external);

    tcp_external.server_start();

    while(1)
        server.parse();

    t.join();

    return 0;
}
