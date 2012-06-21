#include <assert.h>
#include "../dimension_table.h"

//#include <iostream>

using namespace robot::dimension;

struct test0
{
    static_assert(10 == decimical_factor<1>::value, "error0");
};

struct time;
struct size;
struct metre;
struct inch;

template <typename, typename> struct is_same { static constexpr bool s = false; };
template <typename T> struct is_same <T, T> { static constexpr bool s = true; };

struct test1
{
    using f_t = expr<power<size, 2>, expr<power<expr<time>, 3>>, size>;
    using s_t = power<expr<time, size>, 3>;

    using t_t = expr<power<expr<time, size>, 3>, long>;

    using e_t = is_equal<f_t, s_t>;

    static_assert(e_t::value, "error1");
    static_assert(is_equal<dimension_expr::delete_repeats<s_t, t_t>, long>::value, "error2");
};

namespace robot { namespace dimension
{
    template <typename R>
    struct unit_cast<R, inch, metre>
    {
        template <typename T>
        static R cast(const T& t) { return t * 0.0254; }
    };
}}

int main()
{
    using pmeter_t =      phis_value<double, size, metre, decimical_factor<0>>;
    using psantimeter_t = phis_value<double, size, metre, decimical_factor<-2>>;

    using pinch_t =       phis_value<double, size, inch, decimical_factor<0>>;

    pmeter_t p3_0;
    pmeter_t p3_1(12);

    p3_0 = p3_1;

    assert(p3_0.get() == 12);

    psantimeter_t p5_0;

    p5_0 = p3_0;

    assert(p5_0.get() == 1200);

    pinch_t pinch(10);

    p5_0 = pinch;

    assert(p5_0.get() == 25.4);

    return 0;
}
