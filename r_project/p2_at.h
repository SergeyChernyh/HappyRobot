#ifndef __P2_AT_BINDING__
#define __P2_AT_BINDING__

#include <thread>
#include "system_si_metrics.h"
#include "no_system_si_metrics.h"
#include "sonar_subsystem.h"
#include "move_subsystem.h"
#include "p2_at_protocol.h"

namespace robot { namespace p2_at
{

namespace si = system_si_metrics;
namespace no_si = no_system_si_metrics;

struct p2_at_metrics
{
    using millimetre                   = si::apply_factor<si::metre<int16_t>, -3>;
    using p2_at_mobile_sim_length_unit = si::apply_factor<si::metre<int16_t>, -4>;

    using second      = si::second<uint64_t>;
    using millisecond = si::apply_factor<si::metre<int16_t>, -3>;

    using degree = no_si::degree<int16_t>;

    using sonar = subsystem::sonar<millimetre, millisecond>;
    using sonar_bar_t = std::array<sonar, 16>;

    using move_t = subsystem::move_subsystem<millimetre, degree, second>;
};

struct p2_at_subsystems: p2_at_metrics
{
};

template <typename Interface>
class p2_at_device: p2_at_subsystems
{
    class io_ctrl;

    sonar_bar_t sonar_bar;
    move_t move_ctrl;
    io_ctrl io;

    template <uint8_t cmd_num>
    void execute_cmd() { io.send_msg(cmd<cmd_num>()); }

    template <uint8_t cmd_num, typename Arg>
    void execute_cmd(const Arg& arg) { io.send_msg(cmd<cmd_num>(arg)); }

    message<sip> get_sip()
    {
        message<sip> server_info;
        io.recieve_msg(server_info);
        return server_info;
    }

    void parse_sip()
    {
        auto server_info = get_sip();

        auto message_body = at_key<message_body_key>(server_info);
        auto sonars       = at_key<sonar_measurements>(message_body);

        for(auto current_sonar : sonars) {
            auto sonar_num = at_key<sonar_number>(current_sonar);
            auto sonar_val = at_key<sonar_range>(current_sonar);
            at_key<subsystem::value>(sonar_bar[sonar_num]).set(p2_at_mobile_sim_length_unit(sonar_val));
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

        execute_cmd<4>((int16_t)1);
    }

    void change_move_param(const bool& move, const int16_t& cur_vel, const int16_t& cur_r_vel)
    {
        execute_cmd<11>((int16_t)(move ? cur_vel : 0));
        execute_cmd<21>((int16_t)(move ? cur_r_vel : 0));
    }

    void bind_move()
    {
        auto& current_vel =   at_key<subsystem::set_linear_vel >(move_ctrl);
        auto& current_r_vel = at_key<subsystem::set_angular_vel>(move_ctrl);
        auto& do_move =       at_key<subsystem::move           >(move_ctrl);

        auto move_binder = [&, this]() { this->change_move_param(do_move.get(), current_vel.get(), current_r_vel.get()); };

        do_move.add_effector      (move_binder);
        current_vel.add_effector  (move_binder);
        current_r_vel.add_effector(move_binder);
    }

    void bind_subsystems()
    {
        auto& sonar2 = at_key<subsystem::value>(sonar_bar[2]);
        auto& sonar3 = at_key<subsystem::value>(sonar_bar[3]);
        auto& sonar4 = at_key<subsystem::value>(sonar_bar[4]);
        auto& sonar5 = at_key<subsystem::value>(sonar_bar[5]);

        auto checker = [&, this]()
        {
            if(sonar2.get() < 300 || sonar3.get() < 500 || sonar4.get() < 500 || sonar5.get() < 300) {
                this->execute_cmd<11>((int16_t)0);

                if(sonar2.get() + sonar3.get() > sonar4.get() + sonar5.get())
                    this->execute_cmd<21>((int16_t)(15));
                else
                    this->execute_cmd<21>((int16_t)(-15));
            }
            else {
                this->execute_cmd<11>((int16_t)(at_key<subsystem::set_linear_vel> (this->move_ctrl).get()));
                this->execute_cmd<21>((int16_t)(at_key<subsystem::set_angular_vel>(this->move_ctrl).get()));
            }
        };

        sonar3.add_effector(checker);
        sonar4.add_effector(checker);

        sonar2.add_effector(checker);
        sonar5.add_effector(checker);
    }

public:
    p2_at_device(Interface& i):
        io(i)
    {
        start();
        std::thread t([this](){ while(true) this->parse_sip(); });

        at_key<subsystem::move           >(move_ctrl).set(0);
        at_key<subsystem::set_linear_vel >(move_ctrl).set(0);
        at_key<subsystem::set_angular_vel>(move_ctrl).set(0);

        bind_move();
        bind_subsystems();

        at_key<subsystem::set_linear_vel >(move_ctrl).set(1200);
        at_key<subsystem::move           >(move_ctrl).set(1);

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

    template <typename Msg>
    void recieve_msg(Msg& msg)
    {
        update_read_buffer();
        package_creation::parser<Msg>::parse(read_buffer, msg);
    }

    void read_echo() { update_read_buffer(); }
};

}}

#endif //__P2_AT_BINDING__
