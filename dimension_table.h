#ifndef __DIMENSION_TABLE__
#define __DIMENSION_TABLE__

#include <math.h>
#include <stdint.h>

#include <type_traits> 

#include "metaprogramming/concatination.h"

namespace robot { namespace dimension
{
    namespace dimension_expr
    {
        namespace m = metaprogramming;

        template <typename, int>
        struct token;

        template <typename ...>
        struct add_to_expr_;

        template <typename T, typename S>
        using add_to_expr = typename add_to_expr_<T, S>::type;

        template <typename T, int POW, typename Head, typename ...Args>
        struct add_to_expr_<token<T, POW>, m::sequence<Head, Args...>>
        {
            using type = m::concatinate<m::sequence<Head>, add_to_expr<token<T, POW>, m::sequence<Args...>>>;
        };

        template <typename T, int POW0, int POW1, typename ...Args>
        struct add_to_expr_<token<T, POW0>, m::sequence<token<T, POW1>, Args...>>
        {
            using type = m::sequence<token<T, POW0 + POW1>, Args...>;
        };

        template <typename T, int POW0, typename ...Args>
        struct add_to_expr_<token<T, POW0>, m::sequence<token<T, -POW0>, Args...>>
        {
            using type = m::sequence<Args...>;
        };

        template <typename T, int POW>
        struct add_to_expr_<token<T, POW>, m::sequence<>>
        {
            using type = m::sequence<token<T, POW>>;
        };

        template <typename ...>
        struct expr_;

        template <typename ...Args>
        using expr = typename expr_<Args...>::type; 

        template <>
        struct expr_<>
        {
            using type = m::sequence<>;
        };

        template <typename T, typename ...Args>
        struct expr_<T, Args...>
        {
            using type = add_to_expr<token<T, 1>, expr<Args...>>;
        };

        template <typename T, int POW, typename ...Args>
        struct expr_<token<T, POW>, Args...>
        {
            using type = add_to_expr<token<T, POW>, expr<Args...>>;
        };

        template <typename ...Subexpr, typename ...Args>
        struct expr_<m::sequence<Subexpr...>, Args...>: public expr_<Subexpr..., Args...> {};

        template <typename, int>
        struct pow_;

        template <typename T, int POW>
        using pow = typename pow_<T, POW>::type;

        template <typename T, int POW>
        struct pow_
        {
            using type = token<T, POW>;
        };

        template <typename T, int POW0, int POW1>
        struct pow_<token<T, POW0>, POW1>
        {
            using type = token<T, POW0 * POW1>;
        };

        template <typename ...Args, int POW>
        struct pow_<m::sequence<Args...>, POW>
        {
            using type = m::sequence<pow<Args, POW>...>;
        };

        template <typename, typename>
        struct comprasion_;

        template <typename T0, typename T1>
        using comprasion = typename comprasion_<T0, T1>::type;

        template <typename ...Args0, typename ...Args1>
        struct comprasion_<m::sequence<Args0...>, m::sequence<Args1...>>
        {
            using type = expr<m::sequence<Args0...>, pow<m::sequence<Args1...>, -1>>;
        };

        template <typename T>
        struct is_equal_check_
        {
            using type = std::false_type;
        };

        template <>
        struct is_equal_check_<m::sequence<>>
        {
            using type = std::true_type;
        };

        template <typename T>
        using is_equal_check = typename is_equal_check_<T>::type;

        template <typename T0, typename T1>
        struct is_equal_
        {
            using type = is_equal_check<comprasion<T0, T1>>;
        };

        template <typename T0, typename T1>
        using is_equal = typename is_equal_<T0, T1>::type;
    }
    
    template <typename T, int POW>
    using power = dimension_expr::pow<T, POW>;

    template <typename ...Args>
    using expr = dimension_expr::expr<Args...>;
 
    template <typename T0, typename T1>
    using is_equal = dimension_expr::is_equal<expr<T0>, expr<T1>>;

    template <typename, typename>
    struct unit_cast;

    template <typename ValueType, typename Quantity, typename Unit, typename Factor>
    class phis_value
    {
        ValueType value;

        using self_t = phis_value<ValueType, Quantity, Unit, Factor>;

    public:
        phis_value() {}
        phis_value(const ValueType& v): value(v) {}

        void set(const ValueType& v) { value = v; }
        ValueType get() const { return value; }

        phis_value& operator=(const phis_value& p)
        {
            value = p.value;
            return *this;
        }

        template <typename T, typename Q, typename U, typename F>
        phis_value& operator=(const phis_value<T, Q, U, F>& p)
        {
            static_assert(is_equal<Quantity, Q>::value, "phis value assignment error: quontity mismatch");
            value = unit_cast<self_t, phis_value<T, Q, U, F>>::cast(p.get());
            return *this;
        }
    };

    template <typename Q, typename U, typename T0, typename T1, typename F0, typename F1>
    struct unit_cast<phis_value<T0, Q, U, F0>, phis_value<T1, Q, U, F1>>
    {
        static T0 cast(const T1& t) { return t * F1::value / F0::value; }
    };

    template <typename Q0, typename Q1, typename U0, typename U1, typename T0, typename T1, typename F0, typename F1>
    struct unit_cast<phis_value<T0, Q0, U0, F0>, phis_value<T1, Q1, U1, F1>>
    {
        static_assert(is_equal<Q0, Q1>::value, "phis value assignment error: quontity mismatch");
    };

    template <int64_t Pow>
    struct decimical_factor
    {
        static constexpr auto value = pow(10, Pow);
    };
}}

#endif //__DIMENSION_TABLE__
