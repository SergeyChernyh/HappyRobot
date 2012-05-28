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

    sequence_access::at_c<2>(sequence_access::at_key<message_body_key>(x)) = p;
    sequence_access::at_key<chck_sum_key>(x) = chck_sum_calc<cmd>(package<cmd>(p));

    auto m = package<msg>(x);//p, chck_sum_calc<cmd>(package<cmd>(p)));

    robot::package_creation::serialization::serialize_tmp_wrapper<msg>::deserialize(m.get_data(), x);

    std::cout << "---> " << sequence_access::at_c<2>(sequence_access::at_key<message_body_key>(x)) << std::endl;

    i.write((const char*)(m.get_data()), package_creation::size<msg>::value);
}

template <uint8_t n, typename Interface>
void send_vec_cmd(int16_t p, Interface& i)
{
    using namespace p2_at;

    using cmd = command<n, std::vector<int16_t>>;
    using msg = message<cmd>;

    std::vector<int16_t> vec(1, p);

    auto cmd_ = package<cmd>(vec);

    msg x;

    sequence_access::at_key<arg_key>(sequence_access::at_key<message_body_key>(x)) = vec;
    sequence_access::at_key<chck_sum_key>(x) = chck_sum_calc<cmd>(cmd_);
    sequence_access::at_key<byte_count_key>(sequence_access::at_key<message_header_key>(x)) = cmd_.data_size() + package_creation::size<chck_sum_t<cmd>>::value;

    auto m = package<msg>(x);//cmd_.data_size() + package_creation::size<chck_sum_t<cmd>>::value, vec, chck_sum_calc<cmd>(cmd_));

    sequence_access::at_key<arg_key>(sequence_access::at_key<message_body_key>(x))[0] = 333;

    robot::package_creation::serialization::serialize_tmp_wrapper<msg>::deserialize(m.get_data(), x);

    std::cout << "========> " << sequence_access::at_c<2>(sequence_access::at_key<message_body_key>(x))[0] << std::endl;

    i.write((const char*)(m.get_data()), m.data_size()); //TODO
}

int main(int argc, const char* argv[])
{
    robot::tcp_test::TCPInterface tcp;

    tcp.open();

    uint8_t buffer[0xff];

    send_cmd<0>(tcp);

    tcp.read(buffer, 3);

    send_cmd<1>(tcp);
    send_cmd<2>(tcp);

    send_cmd<1>(tcp);

    send_cmd<4 , 1>(tcp);

    send_cmd<11, 1200>(tcp);
    send_cmd<11>(1200, tcp);
    send_vec_cmd<11>(1200, tcp);

    while(true) {
        send_cmd<0>(tcp);
        robot::delay(1000);
    }

    return 0;
}
