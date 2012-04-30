#include <type_traits>

#include <typeinfo>
#include <assert.h>

#include "../sequence.h"

namespace m = robot::metaprogramming;
using namespace robot::sequence_access;

struct s;

using r =
m::sequence
<
    uint8_t,
    m::pair<int, int>,
    std::integral_constant<uint16_t, 8>,
    std::true_type,
    m::pair<uint8_t, char>,
    uint64_t,
    m::pair<s, m::sequence<uint8_t>>,
    uint8_t,
    uint64_t
>;

int main(int argc, const char* argv[])
{
    r rr;

    at_c<1>(rr) = 5;
    assert(at_key<int>(rr) == 5);

    at_c<0>(at_key<s>(rr)) = 3;
    assert(at_c<0>(at_c<6>(rr)) == 3);

    return 0;
}
