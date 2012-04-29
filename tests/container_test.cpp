#include <type_traits>

#include <typeinfo>
#include <assert.h>

#include "../container.h"

using namespace robot::container;
namespace m = robot::metaprogramming;

using r = container<uint8_t, m::pair<int, int>, std::integral_constant<uint16_t, 8>, std::true_type, m::pair<uint8_t, char>, uint64_t, uint8_t, uint64_t>;

int main(int argc, const char* argv[])
{
    r rr;

    at_c<1>(rr) = 5;

    assert(at_key<int>(rr) == 5);

    return 0;
}
