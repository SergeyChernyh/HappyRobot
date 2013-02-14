#include <thread>
#include "device.h"
#include "tcp.h"
#include "device/pioneer_2at.h"

int main()
{
    using namespace robot;

    // common io interface
    auto tcp = wait_for_tcp_connection(INADDR_LOOPBACK, 5200);
    server test_server(tcp);

    // move control regs
    reg<p2at::mm_per_second , READ_FLAG | WRITE_FLAG> preseted_vel;
    reg<p2at::deg_per_second, READ_FLAG | WRITE_FLAG> preseted_rvel;

    using flag = non_dimentional<uint8_t>;

    reg<flag, READ_FLAG | WRITE_FLAG> linear_move_flag;
    reg<flag, READ_FLAG | WRITE_FLAG> angular_move_flag;

    // set regs defalt values
    linear_move_flag.set(flag(0));
    angular_move_flag.set(flag(0));

    // bind regs with functions
    auto& move_function = test_server.get_function_ref(1, 0);
    move_function = move_control_function();

    move_function[0xE] = preseted_vel.make_parameter(0xE);
    move_function[0xF] = preseted_rvel.make_parameter(0xF);
    move_function[0x18] = linear_move_flag.make_parameter(0x18);
    move_function[0x19] = angular_move_flag.make_parameter(0x19);

    // pioneer 2at io interface
    connection p2at_iface(tcp_client(INADDR_LOOPBACK, 8101));

    // bind pioneer 2at actions with regs

    auto pioneer_2at_linear_move =
    [&]()
    {
        int16_t vel =
        linear_move_flag.get().get_value() * preseted_vel.get().get_value();

        std::cout << "set p2at vel == " << vel << std::endl;

        p2at_iface.write(p2at::make_p2_at_cmd<11>(vel));
    };

    auto pioneer_2at_angular_move =
    [&]()
    {
        int16_t rvel =
        angular_move_flag.get().get_value() * preseted_rvel.get().get_value();
        p2at_iface.write(p2at::make_p2_at_cmd<21>(rvel));
    };

    preseted_vel.add_action(pioneer_2at_linear_move);
    preseted_rvel.add_action(pioneer_2at_angular_move);

    linear_move_flag.add_action(pioneer_2at_linear_move);
    angular_move_flag.add_action(pioneer_2at_angular_move);

    // pioneer 2at device start
    auto sync0 = p2at::make_p2_at_cmd<0>();
    auto sync1 = p2at::make_p2_at_cmd<1>();
    auto sync2 = p2at::make_p2_at_cmd<2>();

    p2at::p2_at_msg<p2at::sip> sip_msg;

    p2at_iface.write(sync0); //SYNC 0 send
    p2at_iface.read(sync0); //recieve echo

    p2at_iface.write(sync1); //SYNC 1 send
    p2at_iface.read(sync1); //recieve echo

    p2at_iface.write(sync2); //SYNC 2 send
    p2at_iface.read(get<p2at::msg_head>(sync2)); // recieve message head
    empty_dst e;                                // read and ignore message body
    p2at_iface.read(get<p2at::msg_size>(get<p2at::msg_head>(sync2)), e);

    p2at_iface.write(p2at::make_p2_at_cmd<1>());            // start controller
    p2at_iface.write(p2at::make_p2_at_cmd<4>((uint16_t)1)); // start motors

    auto pioneer_2at_read_data =
    [&]()
    {
        using namespace p2at;
        while(1)
        {
            p2at_iface.read(get<msg_head>(sip_msg)); // recieve message head

            p2at_iface.read
            (
                get<msg_size>(get<msg_head>(sip_msg)), // message body size
                get<msg_body>(sip_msg)                 // recieve message body
            );

            auto sonars =
            get<sonar_measurements>(get<msg_data>((get<msg_body>(sip_msg))));

            p2at_iface.write(sync0); // send PULSE
        }
    };

    std::thread pioneer_2at_thread(pioneer_2at_read_data);

    while(1) {
        test_server.server_package_parse();
    }

    pioneer_2at_thread.join();

    return 0;
}
