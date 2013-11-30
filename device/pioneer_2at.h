#ifndef __PIONEER_2AT__
#define __PIONEER_2AT__

#include "connection.h"
#include "tcp.h"

namespace robot
{
namespace p2at
{

using mm  = dec_factor<-3, metre<int16_t>>;
using deg = degree<int16_t>;
using sec = second<int16_t>;

using mm_per_second = decltype(mm()/sec());
using deg_per_second = decltype(deg()/sec());

struct msg_head;
struct msg_header;
struct msg_size;

struct msg_body;
struct msg_data;
struct msg_chck_sum;

template <typename T>
using p2_at_msg =
std::tuple
<
    pair<msg_head, std::tuple
    <
        pair<msg_header, std::integral_constant<uint16_t, 0xfbfa>>,
        pair<msg_size, uint8_t>
    >>,
    pair<msg_body, std::tuple
    <
        pair<msg_data, T>,
        pair<msg_chck_sum, uint16_t>
    >>
>;

inline uint16_t chck_sum_calc(const char *ptr, uint16_t n)
{
    uint16_t c = 0;
    const uint8_t *p =  (const uint8_t*)ptr;

    while (n > 1) {
        c += (*(p)<<8) | *(p+1);
        n -= 2;
        p += 2;
    }

    if (n > 0)
        c ^= (uint16_t)*(p++);
    return ((c % 0x100) << 8) | (c / 0x100);
}

struct cmd_arg;

template <uint8_t CmdNum, typename T = std::tuple<>>
using p2_at_cmd =
std::tuple
<
    std::integral_constant<uint8_t, CmdNum>,
    at_key
    <
        value_type<T>,
        std::tuple
        <
            pair<std::tuple<>, std::tuple<>>,

            pair<uint16_t,    std::integral_constant<uint8_t, 0x1B>>,
            pair< int16_t,    std::integral_constant<uint8_t, 0x3B>>,
            pair<std::string, std::integral_constant<uint8_t, 0x2B>>
        >
    >,
    pair<cmd_arg, T>
>;

template <uint8_t CmdNum, typename T>
inline p2_at_msg<p2_at_cmd<CmdNum, T>> make_p2_at_cmd(const T& p)
{
    p2_at_msg<p2_at_cmd<CmdNum, T>> res;

    auto& size      = get<msg_size>(get<msg_head>(res));
    auto& body      = get<msg_body>(res);
    auto& cmd       = get<msg_data>(body);
    auto& arg       = get<cmd_arg>(cmd);
    auto& check_sum = get<msg_chck_sum>(body);

    size = calc_size(body);
    arg  = p;
    check_sum = chck_sum_calc(make_buffer(cmd).data, size - calc_size(check_sum));

    return res;
}

template <uint8_t CmdNum>
inline p2_at_msg<p2_at_cmd<CmdNum>> make_p2_at_cmd()
{
    return make_p2_at_cmd<CmdNum>(std::tuple<>());
}

struct status_key;

struct x_pos_key;
struct y_pos_key;    
struct th_pos_key;

struct l_vel_key;    
struct r_vel_key;

struct battery;
struct stall_and_bumpers;
struct control;
struct flags;
struct compass;

struct sonar_measurements;
struct sonar_number;
struct sonar_range;

struct timer;
struct analog;
struct digin;
struct digout;

using sip =
std::tuple
<
    pair<status_key, uint8_t>,

    pair<x_pos_key, uint16_t>,
    pair<y_pos_key, uint16_t>,
    pair<th_pos_key, int16_t>,

    pair<l_vel_key, int16_t>,
    pair<r_vel_key, int16_t>,

    pair<battery, uint8_t>,
    pair<stall_and_bumpers, uint16_t>,
    pair<control, int16_t>,
    pair<flags, uint16_t>,
    pair<compass, uint8_t>,

    pair
    <
        sonar_measurements,
        repeat
        <
            uint8_t,
            std::tuple
            <
                pair<sonar_number, uint8_t>,
                pair<sonar_range, mm>
            >
        >
    >,

    pair<timer, uint16_t>,
    pair<analog, uint8_t>,
    pair<digin, uint8_t>,
    pair<digout, uint8_t>
>;

}}

#endif //__PIONEER_2AT__
