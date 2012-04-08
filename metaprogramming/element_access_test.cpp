#include <type_traits>

#include <boost/mpl/pair.hpp>
#include <typeinfo>

#include "element_access.h"

using namespace robot::metaprogramming_tools;

using p0 = convert_to_pair<int>;
using p1 = convert_to_pair<boost::mpl::pair<int, char>>;

using t0 = at_c<0, int, char, long long>;
using t1 = at_c<1, int, char, long long>;
using t2 = at_c<2, int, char, long long>;

//using t3 = at_c<3, int, char, long long>;

using tt0 = at_c<0, sequence<int, char, long long>>;
using tt1 = at_c<1, sequence<int, char, long long>>;
using tt2 = at_c<2, sequence<int, char, long long>>;

//using tt3 = at_c<3, sequence<int, char, long long>>;

using k0 = pair<char, int>;
using k1 = pair<int, char>;
using k2 = pair<short, long long>;

using seq_0 = sequence<k0, k1, int, k2>;

using f0 = at_key<char , k0, k1, k2>;
using f1 = at_key<int  , k0, k1, k2>;
using f2 = at_key<short, k0, k1, k2>;

using ff0 = at_key<char, seq_0>;
using ff1 = at_key<int, seq_0>;
using ff2 = at_key<short, seq_0>;

struct test_pair
{
    static_assert(std::is_same<p0::first , unspecified>::value, "pair err0");
    static_assert(std::is_same<p0::second, int        >::value, "pair err0");

    static_assert(std::is_same<p1::first , int >::value, "pair err1");
    static_assert(std::is_same<p1::second, char>::value, "pair err1");
};

struct test_access_by_index
{
    static_assert(std::is_same<t0, int>::value, "index err0");
    static_assert(std::is_same<tt0, int>::value, "index err0");

    static_assert(std::is_same<t1, char>::value, "index err1");
    static_assert(std::is_same<tt1, char>::value, "index err1");

    static_assert(std::is_same<t2, long long>::value, "index err2");
    static_assert(std::is_same<tt2, long long>::value, "index err2");
};

struct test_access_by_key
{
    static_assert(std::is_same<f0, int>::value, "err0");
    static_assert(std::is_same<ff0, int>::value, "err0");

    static_assert(std::is_same<f1, char>::value, "err1");
    static_assert(std::is_same<ff1, char>::value, "err1");

    static_assert(std::is_same<f2, long long>::value, "err2");
    static_assert(std::is_same<ff2, long long>::value, "err2");
};

int main(int argc, const char* argv[])
{
    return 0;
}
