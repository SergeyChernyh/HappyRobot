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
    // or i.write((const char*)(package<msg>::data), package_creation::size<msg>::value);
}

template <uint8_t n, int16_t p, typename Interface>
void send_cmd(Interface& i)
{
    using namespace p2_at;

    using msg = message<command<n, constant<int16_t, p>>>;
    auto m = package<msg>();

    i.write((const char*)(m.get_data()), package_creation::size<msg>::value);
    // or i.write((const char*)(package<msg>::data), package_creation::size<msg>::value);
}

template <uint8_t n, typename Interface>
void send_cmd(int16_t p, Interface& i)
{
    using namespace p2_at;

    using cmd = command<n, int16_t>;
    using msg = message<cmd>;

    auto m = package<msg>(p, chck_sum_calc<cmd>(package<cmd>(p)));

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

    auto m = package<msg>(cmd_.data_size() + package_creation::size<chck_sum_t<cmd>>::value, vec, chck_sum_calc<cmd>(cmd_));

    i.write((const char*)(m.get_data()), m.data_size()); //TODO
}

int main(int argc, const char* argv[])
{
    robot::tcp_test::TCPInterface tcp;

    tcp.open();

    send_cmd<0>(tcp);
    send_cmd<1>(tcp);
    send_cmd<2>(tcp);

    send_cmd<1>(tcp);

    send_cmd<4 , 1>(tcp);

    //send_cmd<11, 1200>(tcp);
    //send_cmd<11>(1200, tcp);
    send_vec_cmd<11>(1200, tcp);

    while(true) {
        send_cmd<0>(tcp);
        Sleep(1000);
    }

    return 0;
}
