#include <type_traits>

#include <typeinfo>
#include <assert.h>
#include <iostream>

#include "../sequence.h"

namespace m = robot::metaprogramming;
using namespace robot;
using namespace robot::sequence_access;

struct s;

using r =
m::sequence
<
    uint8_t,
    m::pair<int, int>,
    std::integral_constant<uint16_t, 9>,
    std::true_type,
    m::pair<uint8_t, char>,
    uint64_t,
    m::pair<s, m::sequence<uint8_t>>,
    uint16_t,
    m::sequence<std::integral_constant<uint16_t, 100>, uint32_t>,
    uint64_t
>;

struct print
{
    template <typename T>
    void operator() (const T& t) { std::cout << t << " "; }

    template <typename T, T C>
    void operator() (const std::integral_constant<T, C>& t) { std::cout << C << " "; }

    template <typename ...T>
    void operator() (const m::sequence<T...>& t)
    {
        print i;
        std::cout << "(";
        algorithm::for_each(i, t);
        std::cout << ") ";
    }
};

int main(int argc, const char* argv[])
{
    r rr;

    at_c<1>(rr) = 5;
    assert(at_key<int>(rr) == 5);

    at_c<0>(at_key<s>(rr)) = 3;
    assert(at_c<0>(at_c<6>(rr)) == 3);

    print i;

    i(rr);

    return 0;
}
