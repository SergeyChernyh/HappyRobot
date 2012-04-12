#include <type_traits>

#include "concatination.h"

using namespace robot::metaprogramming_tools;

using p0 = sequence<long, char, int, int*>;
using p1 = sequence<char*, sequence<int>>;

struct test_concatination
{
    static_assert(std::is_same<concatinate<p0, p1>, sequence<long, char, int, int*, char*, sequence<int>>>::value, "concatinate err0");
};

int main(int argc, const char* argv[])
{
    return 0;
}
