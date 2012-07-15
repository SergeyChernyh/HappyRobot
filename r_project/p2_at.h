#ifndef __P2_AT_PROTOCOL_NO_HAIRY_
#define __P2_AT_PROTOCOL_NO_HAIRY_

#include <vector>
#include <string>

#include "package.h"
#include "parameter.h"

namespace robot { namespace p2_at
{
    using namespace metaprogramming;

    template <typename ...Args>
    using pattern = package_creation::pattern<Args...>;

    template <typename ...Args>
    using pack = package_creation::package<Args...>;

    using nothing = pattern<>;

    template <typename T, T C>
    using constant = std::integral_constant<T, C>;

    using head_t = constant<uint16_t, 0xfbfa>;

    constexpr uint16_t byte_swap(uint16_t p) { return ((p % 0x100) << 8) | (p / 0x100); }

    template <typename T>
    uint16_t chck_sum_calc(const package_creation::package<T>& pack)
    {
        const uint8_t *ptr = pack.get_data();
        uint16_t c = 0;
        uint16_t n = (pack.data_size());

        while (n > 1) {
            c += (*(ptr)<<8) | *(ptr+1);
            n -= 2;
            ptr += 2;
        }

        if (n > 0)
            c ^= (uint16_t)*(ptr++);
        return byte_swap(c);
    }

    /////// Message Format ////////////////////////////////

    struct message_header_key;

    struct head_key;
    struct byte_count_key;
    struct message_body_key;
    struct chck_sum_key;

    using message_header =
    pattern
    <
        pair<head_key      , head_t>,
        pair<byte_count_key, uint8_t>
    >;

    template <typename T>
    using message =
    pattern
    <
        pair<message_header_key, message_header>,
        pair<message_body_key  , T>,
        pair<chck_sum_key      , uint16_t>
    >;

    //////////// Client Cmd ///////////////////////////////

    using arg_id_table =
    pattern
    <
        pair<nothing, nothing>,

        pair<uint16_t,    constant<uint8_t, 0x1B>>,
        pair< int16_t,    constant<uint8_t, 0x3B>>,
        pair<std::string, constant<uint8_t, 0x2B>>
    >;

    struct arg_key;

    template <uint8_t cmd_num, typename arg = nothing>
    using command =
    pattern
    <
        constant<uint8_t, cmd_num>,
        metaprogramming::at_key<value_type<arg>, arg_id_table>,
        pair<arg_key, arg>
    >;

    template <uint8_t cmd_num, typename T>
    pack<message<command<cmd_num, T>>> cmd(const T& p)
    {
        using c = command<cmd_num, T>;
        return pack<message<c>>(pack<c>(p).data_size() + sizeof(uint16_t), p, chck_sum_calc<c>(pack<c>(p)));
    }

    template <uint8_t cmd_num>
    pack<message<command<cmd_num>>> cmd()
    {
        using c = command<cmd_num>;
        return pack<message<c>>(pack<c>().data_size() + sizeof(uint16_t), chck_sum_calc<c>(pack<c>()));
    }
    //////////// Sip //////////////////////////////////////

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
    pattern
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
            package_creation::repeat
            <
                pattern
                <
                    pair<sonar_number, uint8_t>,
                    pair<sonar_range, uint16_t>
                >,
                uint8_t
            >
        >,

        pair<timer, uint16_t>,
        pair<analog, uint8_t>,
        pair<digin, uint8_t>,
        pair<digout, uint8_t>
    >;
}

}
#endif //__P2_AT_PROTOCOL_NO_HAIRY_
