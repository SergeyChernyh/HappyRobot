#include <assert.h>
#include "../dimension_table.h"

#include <iostream>

using namespace robot;
using namespace robot::dim;

struct test0
{
    static_assert(10 == decimical_factor<1>::value, "error0");
};

struct time;
struct size;
struct metre;
struct inch;
struct second;
struct hour;

template <typename, typename> struct is_same { static constexpr bool s = false; };
template <typename T> struct is_same <T, T> { static constexpr bool s = true; };

struct test1
{
    using f_t = expr<power<size, 2>, expr<power<expr<time>, 3>>, size>;
    using s_t = power<expr<time, size>, 3>;

    using t_t = expr<power<expr<time, size>, 4>, long>;

    using e_t = is_equal<f_t, s_t>;

    static_assert(e_t::value, "error1");
    static_assert(is_equal<dimension_expr::delete_repeats<s_t, t_t>, expr<time, size, long>>::value, "error2");
};

namespace robot { namespace dim
{
    template <typename R>
    struct unit_cast<R, inch, metre>
    {
        template <typename T>
        static R cast(const T& t) { return t * 0.0254; }
    };
    
    template <typename R>
    struct unit_cast<R, hour, second>
    {
        template <typename T>
        static R cast(const T& t) { return t * 60; }
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

    // hairy casts!
    using dsantimeter_t = dimension<size, metre, decimical_factor<-2>>;
    using dinch_t =       dimension<size, inch, decimical_factor<0>>;

    using cast_3 = cast_details::token_cast_<double, power<dinch_t, 9>, power<dsantimeter_t, 3>>;

    static_assert(std::is_same<typename cast_3::second_tail, robot::metaprogramming::sequence<>>::value, "first_tail error");
    static_assert(std::is_same<typename cast_3::first_tail, robot::metaprogramming::sequence<power<dinch_t, 6>>>::value, "second_tail error");

    assert(cast_3::cast(1) == 16.387064);

    std::cout << cast_3::cast(1) << '\n';
    std::cout << cast_3::cast(10) << '\n';
    std::cout << cast_3::cast(100) << '\n';

    std::cout << cast_3::cast(2) << '\n';

    using cast_4 = cast_details::full_cast<double, expr<power<dinch_t, 2>, dinch_t>, expr<dsantimeter_t, dsantimeter_t, dsantimeter_t>>;

    std::cout << cast_4::cast(1) << '\n';
    std::cout << cast_4::cast(10) << '\n';
    std::cout << cast_4::cast(100) << '\n';

    std::cout << cast_4::cast(2) << '\n';

    ///////////////////////////////////////////

    using dsecond_t = dimension<time, second, decimical_factor<0>>;
    using dhour_t   = dimension<time, hour  , decimical_factor<0>>;

    using santimeter_per_seconds_2 = expr<dsantimeter_t, power<dsecond_t, -2>>;
    using          inch_per_hour_2 = expr<      dinch_t, power<dhour_t  , -2>>;

    using cast_acc = cast_details::full_cast<double, inch_per_hour_2, santimeter_per_seconds_2>;

    std::cout << cast_acc::cast(1) << '\n';
    std::cout << cast_acc::cast(10) << '\n';
    std::cout << cast_acc::cast(100) << '\n';

    std::cout << cast_acc::cast(2) << '\n';

    using psecond_t     = phis_value<double, time, second, decimical_factor<0 >>;
    using phour_t       = phis_value<double, time, hour  , decimical_factor<0 >>;
    using psantimeter_t = phis_value<double, size, metre , decimical_factor<-2>>;
    using pinch_t       = phis_value<double, size, inch  , decimical_factor<0 >>;

    phour_t h(1);
    pinch_t i(1);

    auto a = i / h / h;

    phis_value_<double, expr<size, power<time, -2>>, expr<dsantimeter_t, power<dsecond_t, -2>>> b;

    b = a;

    std::cout << b.get() << '\n';

    return 0;
}
