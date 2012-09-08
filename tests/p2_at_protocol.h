#ifndef __P2_AT_PROTOCOL__
#define __P2_AT_PROTOCOL__

#include <vector>
#include <string>

#include "package.h"

namespace robot { namespace p2_at
{
    template <typename Key, typename T>
    using pair = metaprogramming::pair<Key, T>;

    template <typename ...Args>
    using pattern = package_creation::pattern<Args...>;

    template <typename ...Args>
    using pack = package_creation::package<Args...>;

    using nothing = pattern<>;

    template <typename T, T C>
    using constant = std::integral_constant<T, C>;

    using head_t = constant<uint16_t, 0xfbfa>;

    template <typename T>
    using calc_byte_count_options =
    pattern
    <
        pair<std::false_type, uint8_t>,
        pair<std::true_type , metaprogramming::byte_count<uint8_t, T, uint16_t>>
    >;

    template <typename T>
    using byte_count_t = metaprogramming::at_key<metaprogramming::is_const_size<T>, calc_byte_count_options<T>>;

    /////////// Chck Sum calc /////////////////////////////

    constexpr uint16_t byte_swap(uint16_t p) { return ((p % 0x100) << 8) | (p / 0x100); }

    constexpr uint16_t chck_sum_calc(uint16_t sum) { return sum; }

    constexpr uint16_t chck_sum_calc(uint16_t sum, uint8_t c) { return sum ^ c; }

    template <typename ...Args>
    constexpr uint16_t chck_sum_calc(uint16_t sum, uint8_t c0, uint8_t c1, Args... args)
    {
        return chck_sum_calc(sum + c1 + c0 * 0x100, args...);
    }

    template <typename T>
    struct chck_sum_calc_t
    {
        using type = metaprogramming::unspecified;
    };

    template <typename ...Args>
    struct chck_sum_calc_t<pattern<Args...>>
    {
        using type = constant<uint16_t, byte_swap(chck_sum_calc(0, Args::value...))>;
    };

    template <typename T>
    using check_sum_calc_options = 
    pattern
    <
        pair<std::false_type, uint16_t>,
        pair<std::true_type , typename chck_sum_calc_t<metaprogramming::serialize<T>>::type>
    >;

    template <typename T>
    using chck_sum_t = metaprogramming::at_key<metaprogramming::is_const<T>, check_sum_calc_options<T>>;

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

    template <typename T>
    using message_header =
    pattern
    <
        pair<head_key      , head_t>,
        pair<byte_count_key, byte_count_t<T>>
    >;

    template <typename T>
    using message =
    pattern
    <
        pair<message_header_key, message_header<T>>,
        pair<message_body_key  , T>,
        pair<chck_sum_key      , chck_sum_t<T>>
    >;

    //////////// Client Cmd ///////////////////////////////

    using arg_id_table =
    pattern
    <
        pair<nothing, nothing>,

        pair<uint16_t,    constant<uint8_t, 0x1B>>,
        pair< int16_t,    constant<uint8_t, 0x3B>>,
        pair<std::string, constant<uint8_t, 0x2B>>,

        pair<std::vector<int16_t>, constant<uint8_t, 0x3B>>, // TODO DELETE
        pair<std::vector<package_creation::serialization::any>, constant<uint8_t, 0x3B>> // TODO DELETE
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
#endif //__P2_AT_PROTOCOL__
