#include <type_traits>

#include <typeinfo>
#include <assert.h>

#include "data_sequence.h"

using namespace robot::metaprogramming_tools;

using t = data_sequence<uint8_t, pair<int, int>, std::integral_constant<uint16_t, 8>, std::true_type, pair<uint8_t, char>, uint64_t>;

using uint8_accessor = element_access::data_accessor<uint8_t, t>;
using int_accessor = element_access::data_accessor<int, t>;

using res_t = uint8_accessor::res_t;
using element_t = uint8_accessor::element_t;

struct assert
{
    static_assert(std::is_same<char, res_t>::value, "assert_0");
    static_assert(std::is_same<element_t, data_sequence_element<std::false_type, uint8_t, char>>::value, "assert_1");

    static_assert(std::is_same<int, int_accessor::res_t>::value, "assert_2");
    static_assert(std::is_same<int_accessor::element_t, data_sequence_element<std::false_type, int, int>>::value, "assert_3");
};

int main(int argc, const char* argv[])
{
    t tt;
    uint8_accessor::get(tt) = 5;

    assert(get<uint8_t>(tt) == 5);

    return 0;
}

