#ifndef __DIMENSION_TABLE__
#define __DIMENSION_TABLE__

#include <math.h>
#include <stdint.h>

#include <type_traits> 

#include "metaprogramming/select.h"

namespace robot { namespace dimension
{
    namespace dimension_expr
    {
        namespace m = metaprogramming;

        template <typename, int>
        struct token;

        template <bool add_to_tail, typename ...>
        struct add_to_expr_;

        template <bool add_to_tail, typename T, typename S>
        using add_to_expr = typename add_to_expr_<add_to_tail, T, S>::type;

        template <bool add_to_tail, typename T, int POW, typename Head, typename ...Args>
        struct add_to_expr_<add_to_tail, token<T, POW>, m::sequence<Head, Args...>>
        {
            using type = m::concatinate<m::sequence<Head>, add_to_expr<add_to_tail, token<T, POW>, m::sequence<Args...>>>;
        };

        template <bool add_to_tail, typename T, int POW0, int POW1, typename ...Args>
        struct add_to_expr_<add_to_tail, token<T, POW0>, m::sequence<token<T, POW1>, Args...>>
        {
            using type = m::sequence<token<T, POW0 + POW1>, Args...>;
        };

        template <bool add_to_tail, typename T, int POW0, typename ...Args>
        struct add_to_expr_<add_to_tail, token<T, POW0>, m::sequence<token<T, -POW0>, Args...>>
        {
            using type = m::sequence<Args...>;
        };

        template <typename T, int POW>
        struct add_to_expr_<true, token<T, POW>, m::sequence<>>
        {
            using type = m::sequence<token<T, POW>>;
        };

        template <bool, typename ...>
        struct expr_;

        template <typename ...Args>
        using expr = typename expr_<true, Args...>::type; 

        template <>
        struct expr_<true>
        {
            using type = m::sequence<>;
        };

        template <bool add_to_tail, typename T, typename ...Args>
        struct expr_<add_to_tail, T, Args...>
        {
            using type = add_to_expr<add_to_tail, token<T, 1>, expr<Args...>>;
        };

        template <bool add_to_tail, typename T, int POW, typename ...Args>
        struct expr_<add_to_tail, token<T, POW>, Args...>
        {
            using type = add_to_expr<add_to_tail, token<T, POW>, expr<Args...>>;
        };

        template <bool add_to_tail, typename ...Subexpr, typename ...Args>
        struct expr_<add_to_tail, m::sequence<Subexpr...>, Args...>: public expr_<add_to_tail, Subexpr..., Args...> {};

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

    template <typename R, typename U0, typename U1>
    struct unit_cast;

    template <typename R, typename U>
    struct unit_cast<R, U, U>
    {
        template <typename T>
        static R cast(const T& t) { return t; }
    };

    template <typename, typename, typename>
    struct dimension;

    template <typename R, typename U0, typename U1>
    struct dimension_cast;

    template <typename R, typename Q, typename U0, typename U1, typename F0, typename F1>
    struct dimension_cast<R, dimension<Q, U0, F0>, dimension<Q, U1, F1>>
    {
        template <typename T>
        static R cast(const T& t) { return unit_cast<R, U0, U1>::cast(t) * F0::value / F1::value; }
    };

    template <typename, typename, typename>
    struct phis_value_;

    template <typename ValueType, typename Quantity, typename Unit, typename Factor>
    using phis_value = phis_value_<ValueType, Quantity, dimension<Quantity, Unit, Factor>>;

    template <typename ValueType, typename Quantity, typename Dimension>
    class phis_value_
    {
        ValueType value;

        using self_t = phis_value_<ValueType, Quantity, Dimension>;

        template <typename V, typename Q, typename D>
        class convertor
        {
        public:
            template <typename>
            struct quantity_;

            template <typename Q0, typename U0, typename F0>
            struct quantity_<dimension<Q0, U0, F0>>
            {
                using type = Q0;
            };

            template <typename T, int C>
            struct quantity_<dimension_expr::token<T, C>>: public quantity_<T> {};

            template <typename T>
            using quantity = typename quantity_<T>::type;

            template <typename T, typename Seq0, typename Seq1>
            struct separator
            {
                template <typename A>
                using equal = std::integral_constant<bool, std::is_same<quantity<T>, quantity<A>>::value>;

                template <typename A>
                using not_equal = std::integral_constant<bool, !std::is_same<quantity<T>, quantity<A>>::value>;

                using convertion_seq_pair =
                metaprogramming::sequence
                <
                    metaprogramming::select<equal, Seq0>,
                    metaprogramming::select<equal, Seq1>
                >;

                using convertion_tail =
                metaprogramming::sequence
                <
                    metaprogramming::select<not_equal, Seq0>,
                    metaprogramming::select<not_equal, Seq1>
                >;
            };

            template <typename, typename, typename>
            struct convertion_table;
        };

    public:
        phis_value_() {}
        phis_value_(const ValueType& v): value(v) {}

        void set(const ValueType& v) { value = v; }
        ValueType get() const { return value; }

        phis_value_& operator=(const phis_value_& p)
        {
            value = p.value;
            return *this;
        }

        template <typename V, typename Q, typename D>
        phis_value_& operator=(const phis_value_<V, Q, D>& p)
        {
            static_assert(is_equal<Quantity, Q>::value, "phis value assignment error: quontity mismatch");
            using cast = convertor<V, Q, D>;
            value = dimension_cast<ValueType, D, Dimension>::cast(p.get());
            return *this;
        }
    };

    template <int64_t Pow>
    struct decimical_factor
    {
        static constexpr auto value = pow(10, Pow);
    };
}}

#endif //__DIMENSION_TABLE__
