#ifndef __P2_AT_BINDING__
#define __P2_AT_BINDING__

#include <thread>
#include "sonar_subsystem.h"
#include "p2_at.h"

namespace robot { namespace p2_at
{

namespace si = system_si_metrics;

template <typename Interface>
struct p2_at_device
{
    using millimetre                   = si::apply_factor<si::metre<int16_t>, -3>;
    using p2_at_mobile_sim_length_unit = si::apply_factor<si::metre<int16_t>, -4>;

    using sonar = subsystem::sonar<millimetre, int64_t>;
    using sonar_bar_t = std::array<sonar, 16>;

    class io_ctrl;

    sonar_bar_t sonar_bar;
    io_ctrl io;

    template <uint8_t cmd_num>
    void execute_cmd() { io.send_msg(cmd<cmd_num>()); }

    template <uint8_t cmd_num, typename Arg>
    void execute_cmd(const Arg& arg) { io.send_msg(cmd<cmd_num>(arg)); }

    void parse_sip()
    {
        namespace a = sequence_access;
        auto server_info = io.recieve_sip();

        auto message_body = a::at_key<message_body_key>(server_info);
        auto sonars       = a::at_key<sonar_measurements>(message_body);

        for(auto current_sonar : sonars) {
            auto sonar_num = a::at_key<sonar_number>(current_sonar);
            auto sonar_val = a::at_key<sonar_range>(current_sonar);
            a::at_key<subsystem::value>(sonar_bar[sonar_num]).set(p2_at_mobile_sim_length_unit(sonar_val));
        }

        execute_cmd<0>();
    };

    void read_echo() { io.read_echo(); }

    void start()
    {
        execute_cmd<0>(); read_echo();
        execute_cmd<1>(); read_echo();
        execute_cmd<2>(); read_echo();

        execute_cmd<1>();

        execute_cmd<4 >((int16_t)1);
        execute_cmd<11>((int16_t)1200);
    }

    void bind_subsystems()
    {
        auto checker = [this](const millimetre& v) { if(v.get() < 500) this->execute_cmd<11>((int16_t)0); };

        sequence_access::at_key<subsystem::value>(sonar_bar[3]).add_effector(checker);
        sequence_access::at_key<subsystem::value>(sonar_bar[4]).add_effector(checker);
    }

public:
    p2_at_device(Interface& i):
        io(i)
    {
        start();
        std::thread t([this](){ while(true) this->parse_sip(); });
        bind_subsystems();
        t.join();
    }
};

template <typename Interface>
class p2_at_device<Interface>::io_ctrl
{
    Interface& interface;

    static constexpr size_t buffer_size = 0xff;
    uint8_t read_buffer[buffer_size];

    void update_read_buffer()
    {
        using namespace robot::package_creation;

        uint8_t byte_readed = 0;

        constexpr size_t header_size = size<message_header>::value;

        interface.read(read_buffer, header_size);
        parser<message_header>::parse(read_buffer, byte_readed);
        interface.read(read_buffer + header_size, byte_readed);
    }

public:
    io_ctrl(Interface& i): interface(i) {}

    template <typename Msg>
    void send_msg(const Msg& msg)
    {
        interface.write((const char*)(msg.get_data()), msg.data_size());
    }

    message<sip> recieve_sip()
    {
        update_read_buffer();
        message<sip> server_info;
        package_creation::parser<message<sip>>::parse(read_buffer, server_info);
        return server_info;
    }

    void read_echo() { update_read_buffer(); }
};

}}

#endif //__P2_AT_BINDING__
