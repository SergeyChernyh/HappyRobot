#ifndef __P2_AT_PROTOCOL__
#define __P2_AT_PROTOCOL__

#include <vector>

#include "package.h"

namespace robot { namespace p2_at
{
    using namespace metaprogramming_tools;

    using nothing = sequence<>;

    template <typename T, T C>
    using constant = std::integral_constant<T, C>;

    using head_t = constant<uint16_t, 0xfbfa>;

    template <typename T>
    using calc_byte_count_options =
    sequence
    <
        pair<std::false_type, uint8_t>,
        pair<std::true_type , byte_count<uint8_t, T, uint16_t>>
    >;

    template <typename T>
    using byte_count_t = at_key<is_const_size<T>, calc_byte_count_options<T>>;

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
        using type = unspecified;
    };

    template <typename ...Args>
    struct chck_sum_calc_t<sequence<Args...>>
    {
        using type = constant<uint16_t, byte_swap(chck_sum_calc(0, Args::value...))>;
    };

    template <typename T>
    using check_sum_calc_options = 
    sequence
    <
        pair<std::false_type, uint16_t>,
        pair<std::true_type , typename chck_sum_calc_t<serialize<T>>::type>
    >;

    template <typename T>
    using chck_sum_t = at_key<is_const<T>, check_sum_calc_options<T>>;

    template <typename T>
    uint16_t chck_sum_calc(const package_creation::package<T>& pack)
    {
        const uint8_t *ptr = pack.data;
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

    struct head_key{};
    struct byte_count_key{};
    struct message_body_key{};
    struct chck_sum_key{};

    template <typename T>
    using message =
    sequence
    <
        pair<head_key        , head_t>,
        pair<byte_count_key  , byte_count_t<T>>,
        pair<message_body_key, T>,
        pair<chck_sum_key    , chck_sum_t<T>>
    >;

    //////////// Client Cmd ///////////////////////////////

    using arg_id_table =
    sequence
    <
        pair<nothing, nothing>,

        pair<uint16_t,    constant<uint8_t, 0x1B>>,
        pair< int16_t,    constant<uint8_t, 0x3B>>,
        pair<std::string, constant<uint8_t, 0x2B>>,

        pair<std::vector<int16_t>, constant<uint8_t, 0x3B>> // TODO DELETE
    >;

    template <uint8_t cmd_num, typename arg = nothing>
    using command =
    sequence
    <
        constant<uint8_t, cmd_num>,
        at_key<value_type<arg>, arg_id_table>,
        arg
    >;
}

}
#endif //__P2_AT_PROTOCOL__
