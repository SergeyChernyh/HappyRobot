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
};

struct p2_at_subsystems: p2_at_metrics
{
    using sonar = subsystem::sonar<millimetre, millisecond>;
    using sonar_bar_t = std::array<sonar, 16>;

    using move_t = subsystem::move_subsystem<millimetre, degree, second>;

    sonar_bar_t sonar_bar;
    move_t move_ctrl;

    template <size_t quater>
    void set_sonars_positions_quater()
    {
        using namespace subsystem;
                     /*
                     ^ X
          sonars 0-3 | sonars 4-7
           Y    q0   |    q1    
           <---------|----------
                q2   |    q3
        sonars 12-15 | sonars 8-11
                    */

        static_assert(quater <= 3, "quater <= 3");
        int16_t x_factor = quater == 0 || quater == 1;
        int16_t y_factor = quater == 0 || quater == 2;

        size_t base_sonar_num = quater * 4;
    //0
        at_key<x_pos>(sonar_bar[0 + base_sonar_num]).set(145 * x_factor);
        at_key<y_pos>(sonar_bar[0 + base_sonar_num]).set(130 * y_factor);
        at_key<z_pos>(sonar_bar[0 + base_sonar_num]).set(  0);

        at_key<x_vect_pos>(sonar_bar[0 + base_sonar_num]).set(0 * x_factor);
        at_key<y_vect_pos>(sonar_bar[0 + base_sonar_num]).set(1 * y_factor);
        at_key<z_vect_pos>(sonar_bar[0 + base_sonar_num]).set(0);
    //1
        at_key<x_pos>(sonar_bar[1 + base_sonar_num]).set(115 * x_factor);
        at_key<y_pos>(sonar_bar[1 + base_sonar_num]).set(185 * y_factor);
        at_key<z_pos>(sonar_bar[1 + base_sonar_num]).set(  0);

        at_key<x_vect_pos>(sonar_bar[1 + base_sonar_num]).set(100 * x_factor);
        at_key<y_vect_pos>(sonar_bar[1 + base_sonar_num]).set(119 * y_factor);
        at_key<z_vect_pos>(sonar_bar[1 + base_sonar_num]).set(  0);
    //2
        at_key<x_pos>(sonar_bar[2 + base_sonar_num]).set(220 * x_factor);
        at_key<y_pos>(sonar_bar[2 + base_sonar_num]).set( 80 * y_factor);
        at_key<z_pos>(sonar_bar[2 + base_sonar_num]).set(  0);

        at_key<x_vect_pos>(sonar_bar[2 + base_sonar_num]).set(100 * x_factor);
        at_key<y_vect_pos>(sonar_bar[2 + base_sonar_num]).set( 58 * y_factor);
        at_key<z_vect_pos>(sonar_bar[2 + base_sonar_num]).set(  0);
    //3
        at_key<x_pos>(sonar_bar[3 + base_sonar_num]).set(240 * x_factor);
        at_key<y_pos>(sonar_bar[3 + base_sonar_num]).set( 25 * y_factor);
        at_key<z_pos>(sonar_bar[3 + base_sonar_num]).set(  0);

        at_key<x_vect_pos>(sonar_bar[3 + base_sonar_num]).set(100 * x_factor);
        at_key<y_vect_pos>(sonar_bar[3 + base_sonar_num]).set( 18 * y_factor);
        at_key<z_vect_pos>(sonar_bar[3 + base_sonar_num]).set(  0);
    }

    void set_sonars_positions()
    {
        set_sonars_positions_quater<0>();
        set_sonars_positions_quater<1>();
        set_sonars_positions_quater<2>();
        set_sonars_positions_quater<3>();
    }

    p2_at_subsystems()
    {
        set_sonars_positions();
    }
};

template <typename Interface>
class p2_at_device: p2_at_subsystems
{
    class io_ctrl;
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

    void parse_sip(const message<sip>& server_info)
    {
        parse_sonars(server_info);
    }

    void parse_sonars(const message<sip>& server_info)
    {
        auto message_body = at_key<message_body_key>(server_info);
        auto sonars       = at_key<sonar_measurements>(message_body);

        for(auto current_sonar : sonars) {
            auto sonar_num = at_key<sonar_number>(current_sonar);
            auto sonar_val = at_key<sonar_range>(current_sonar);
            at_key<subsystem::value>(sonar_bar[sonar_num]).set(p2_at_mobile_sim_length_unit(sonar_val));
        }
    }

    void read_echo() { io.read_echo(); }

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

public:
    p2_at_device(Interface& i):
        io(i)
    {
        execute_cmd<0>(); read_echo();
        execute_cmd<1>(); read_echo();
        execute_cmd<2>(); read_echo();

        execute_cmd<1>();

        execute_cmd<4 >((int16_t)1);

        bind_move();

    }

    void start()
    {
        while(true) {
            parse_sip(get_sip());
            execute_cmd<0>();
        }
    }

          p2_at_subsystems& get_subsystems()       { return *this; }
    const p2_at_subsystems& get_subsystems() const { return *this; }
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
