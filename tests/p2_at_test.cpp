#include "p2_at_protocol.h"
#include "tcp.h"

#include <iostream>
#include <vector>
#include <typeinfo>

using namespace robot;
using namespace robot::package_creation;

template <uint8_t n, typename Interface>
void send_cmd(Interface& i)
{
    using namespace p2_at;

    using msg = message<command<n>>;
    auto m = package<msg>();
    i.write((const char*)(m.get_data()), package_creation::size<msg>::value);
}

template <uint8_t n, int16_t p, typename Interface>
void send_cmd(Interface& i)
{
    using namespace p2_at;

    using msg = message<command<n, constant<int16_t, p>>>;
    auto m = package<msg>();

    i.write((const char*)(m.get_data()), package_creation::size<msg>::value);
}

template <uint8_t n, typename Interface>
void send_cmd(int16_t p, Interface& i)
{
    using namespace p2_at;

    using cmd = command<n, int16_t>;
    using msg = message<cmd>;

    msg x;

    at_c<2>(at_key<message_body_key>(x)) = p;
    at_key<chck_sum_key>(x) = chck_sum_calc<cmd>(package<cmd>(p));

    auto m = package<msg>(x);//p, chck_sum_calc<cmd>(package<cmd>(p)));

    robot::package_creation::serialization::serialize_tmp_wrapper<msg>::deserialize(m.get_data(), x);

    std::cout << "---> " << at_c<2>(at_key<message_body_key>(x)) << std::endl;

    i.write((const char*)(m.get_data()), package_creation::size<msg>::value);
}

template <uint8_t n, typename Interface>
void send_vec_cmd(int16_t p, Interface& i)
{
    using namespace p2_at;

    using cmd = command<n, std::vector<package_creation::serialization::any>>;
    using msg = message<cmd>;

    std::vector<package_creation::serialization::any> vec(1, package_creation::serialization::any(p));

    auto cmd_ = package<cmd>(vec);

    msg x;

    at_key<arg_key>(at_key<message_body_key>(x)) = vec;
    at_key<chck_sum_key>(x) = chck_sum_calc<cmd>(cmd_);
    at_key<byte_count_key>(at_key<message_header_key>(x)) = cmd_.data_size() + package_creation::size<chck_sum_t<cmd>>::value;

    auto m = package<msg>(x);//cmd_.data_size() + package_creation::size<chck_sum_t<cmd>>::value, vec, chck_sum_calc<cmd>(cmd_));

    //at_key<arg_key>(at_key<message_body_key>(x))[0] = 333;

    robot::package_creation::serialization::serialize_tmp_wrapper<msg>::deserialize(m.get_data(), x);

    //std::cout << "========> " << at_c<2>(at_key<message_body_key>(x))[0] << std::endl;

    i.write((const char*)(m.get_data()), m.data_size()); //TODO
}

int main(int argc, const char* argv[])
{
    robot::tcp_test::TCPInterface tcp;

    tcp.client_connect();

    uint8_t buffer[0xff];

    auto read_message =
    [&]()
    {
        using header = p2_at::message_header<metaprogramming::unspecified>;
        uint8_t byte_count = 0;
        tcp.read(buffer, size<header>::value);
        parser<header>::parse(buffer, byte_count);
        tcp.read(buffer + size<header>::value, byte_count);
    };

    send_cmd<0>(tcp); read_message();
    send_cmd<1>(tcp); read_message();
    send_cmd<2>(tcp); read_message();

    send_cmd<1>(tcp);

    send_cmd<4 , 1>(tcp);

    send_cmd<11, 1200>(tcp);
    send_cmd<11>(1200, tcp);
    send_vec_cmd<11>(1200, tcp);

    while(true) {
        read_message();
        p2_at::message<p2_at::sip> server_info;
        parser<p2_at::message<p2_at::sip>>::parse(buffer, server_info);

        auto sonars = at_key<p2_at::sonar_measurements>(at_key<p2_at::message_body_key>(server_info));

        std::cout << "Sonars\n";
        for(size_t i = 0; i < sonars.size(); i++)
            std::cout << at_key<p2_at::sonar_number>(sonars[i]) << ") " << at_key<p2_at::sonar_range>(sonars[i]) << std::endl;

        send_cmd<0>(tcp);
    }

    return 0;
}
