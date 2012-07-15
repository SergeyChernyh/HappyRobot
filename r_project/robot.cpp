#include "p2_at.h"
#include "tcp.h"

#include <iostream>
#include <vector>
#include <typeinfo>

using namespace robot;

template <typename I, typename T>
void send_msg(I& i, const T& t) { i.write((const char*)(t.get_data()), t.data_size()); }

int main(int argc, const char* argv[])
{
    robot::tcp_test::TCPInterface tcp;

    tcp.open();

    uint8_t buffer[0xff];

    auto read_message =
    [&]()
    {
        using namespace robot::package_creation;
        using header = p2_at::message_header;
        uint8_t byte_count = 0;
        tcp.read(buffer, size<header>::value);
        parser<header>::parse(buffer, byte_count);
        tcp.read(buffer + size<header>::value, byte_count);
    };

    send_msg(tcp, p2_at::cmd<0>()); read_message();
    send_msg(tcp, p2_at::cmd<1>()); read_message();
    send_msg(tcp, p2_at::cmd<2>()); read_message();

    send_msg(tcp, p2_at::cmd<1>());

    send_msg(tcp, p2_at::cmd<4>((int16_t)1));

    send_msg(tcp, p2_at::cmd<11>((int16_t)1200));

    while(true) {
        namespace a = sequence_access;

        read_message();
        p2_at::message<p2_at::sip> server_info;
        package_creation::parser<p2_at::message<p2_at::sip>>::parse(buffer, server_info);

        auto sonars = a::at_key<p2_at::sonar_measurements>(a::at_key<p2_at::message_body_key>(server_info));

        std::cout << "Sonars\n";
        for(size_t i = 0; i < sonars.size(); i++)
            std::cout << a::at_key<p2_at::sonar_number>(sonars[i]) << ") " << a::at_key<p2_at::sonar_range>(sonars[i]) << std::endl;

        send_msg(tcp, p2_at::cmd<0>());
    }

    return 0;
}
