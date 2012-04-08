#include <type_traits>

#include <boost/mpl/pair.hpp>

#include "serialization.h"

using namespace robot::metaprogramming_tools;

using q0 = std::integral_constant<uint32_t, 0xff00ff00>;
using q1 = std::integral_constant<uint16_t, 0x0>;
using q2 = std::integral_constant<uint16_t, 0xee00>;
using q3 = std::integral_constant<uint8_t, 0x88>;

using p0 = sequence<q0, q1, q2>;
using p1 = sequence<q3>;

struct test
{
    static_assert(
    std::is_same
    <
        serialize<p0, p1>,
        sequence
        <
            std::integral_constant<uint8_t, 0x0>,
            std::integral_constant<uint8_t, 0xff>,
            std::integral_constant<uint8_t, 0x0>,
            std::integral_constant<uint8_t, 0xff>,
            std::integral_constant<uint8_t, 0x0>,
            std::integral_constant<uint8_t, 0x0>,
            std::integral_constant<uint8_t, 0x0>,
            std::integral_constant<uint8_t, 0xee>,
            std::integral_constant<uint8_t, 0x88>
        >
    >::value, "err0");
};

int main(int argc, const char* argv[])
{
    return 0;
}
