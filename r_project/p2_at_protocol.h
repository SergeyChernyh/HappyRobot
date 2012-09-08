#ifndef __P2_AT_PROTOCOL_NO_HAIRY_
#define __P2_AT_PROTOCOL_NO_HAIRY_

#include <vector>
#include <string>

#include "protocol_definition_names.h"

namespace robot { namespace p2_at
{
    using namespace protocol_definition_names;

    inline uint16_t chck_sum_calc(const uint8_t *ptr, uint16_t n)
    {
        uint16_t c = 0;

        while (n > 1) {
            c += (*(ptr)<<8) | *(ptr+1);
            n -= 2;
            ptr += 2;
        }

        if (n > 0)
            c ^= (uint16_t)*(ptr++);
        return ((c % 0x100) << 8) | (c / 0x100);
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
        pair<head_key      , constant<uint16_t, 0xfbfa>>,
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
        metaprogramming::at_key<metaprogramming::value_type<arg>, arg_id_table>,
        pair<arg_key, arg>
    >;

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
                uint8_t,
                pattern
                <
                    pair<sonar_number, uint8_t>,
                    pair<sonar_range, uint16_t>
                >
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
