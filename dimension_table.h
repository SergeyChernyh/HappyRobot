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

        template <typename T, int POW>
        struct add_to_expr_<false, token<T, POW>, m::sequence<>>
        {
            using type = m::sequence<>;
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
            using type = add_to_expr<true, token<T, 1>, expr<Args...>>;
        };

        template <typename T, int POW, typename ...Args>
        struct expr_<token<T, POW>, Args...>
        {
            using type = add_to_expr<true, token<T, POW>, expr<Args...>>;
        };

        template <typename ...Subexpr, typename ...Args>
        struct expr_<m::sequence<Subexpr...>, Args...>: expr_<Subexpr..., Args...> {};

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

        template <typename ...>
        struct delete_repeats_;

        template <typename ...Args>
        using delete_repeats = typename delete_repeats_<Args...>::type;

        template <typename T, typename S>
        struct add_check_: add_to_expr_<false, pow<T, -1>, S> {};

        template <typename T, typename S>
        using add_check = typename add_check_<T, S>::type;

        template <typename T, typename S>
        struct delete_repeats_<T, S>
        {
            using type = add_check<T, S>;
        };

        template <typename Head, typename ...Subexpr, typename S>
        struct delete_repeats_<m::sequence<Head, Subexpr...>, S>
        {
            using type = add_check<Head, delete_repeats<m::sequence<Subexpr...>, S>>;
        };

        template <typename S>
        struct delete_repeats_<m::sequence<>, S>
        {
            using type = S;
        };
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

    namespace cast_details
    {
        namespace m = metaprogramming;

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

        template <typename>
        struct pos_pow;

        template <typename T, int C>
        struct pos_pow<dimension_expr::token<T, C>>
        {
            static const bool value = (C > 0);
        };

        template <typename T, typename Seq>
        struct separator
        {
            template <typename A>
            using equal = std::integral_constant<bool, std::is_same<quantity<T>, quantity<A>>::value>;

            template <typename A>
            using pos_equal = std::integral_constant<bool, equal<A>::value && pos_pow<A>::value>;

            template <typename A>
            using neg_equal = std::integral_constant<bool, equal<A>::value && !pos_pow<A>::value>;

            template <typename A>
            using not_equal = std::integral_constant<bool, !std::is_same<quantity<T>, quantity<A>>::value>;

            using convertion_seq     = m::select<equal    , Seq>;
            using pos_convertion_seq = m::select<pos_equal, Seq>;
            using neg_convertion_seq = m::select<neg_equal, Seq>;
            using convertion_tail    = m::select<not_equal, Seq>;
        };

        template <typename R, typename U0, typename U1>
        struct reverse_cast
        {
            template <typename T>
            static R cast(const T& t) { return 1 / dimension_cast<R, U0, U1>::cast(1 / t); }
        };

        template <typename R, typename T0, typename T1, int C, bool SIGN>
        struct repeat_cast_;

        template <typename R, typename T0, typename T1, int C>
        using repeat_cast = typename repeat_cast_<R, T0, T1, C, (C > 0)>::type;

        template <typename R, typename T0, typename T1, int C>
        struct repeat_cast_<R, T0, T1, C, 1>
        {
            using type =
            m::concatinate
            <
                m::sequence<dimension_cast<R, T0, T1>>,
                repeat_cast<R, T0, T1, C - 1>
            >;
        };

        template <typename R, typename T0, typename T1, int C>
        struct repeat_cast_<R, T0, T1, C, 0>
        {
            using type =
            m::concatinate
            <
                m::sequence<reverse_cast<R, T0, T1>>,
                repeat_cast<R, T0, T1, C + 1>
            >;
        };

        template <typename R, typename T0, typename T1, bool SIGN>
        struct repeat_cast_<R, T0, T1, 0, SIGN>
        {
            using type = m::sequence<>;
        };

        template <typename, typename, typename>
        struct add_to_cast_sequence_        {
            using type = m::sequence<>;//repeat_cast<V, T0, T1, C>;

            using first_tail  = m::sequence<>;
            using second_tail = m::sequence<>;
        };;

        template <typename V, typename T0, typename T1, int C>
        struct add_to_cast_sequence_<V, dimension_expr::token<T0, C>, dimension_expr::token<T1, C>>
        {
            using type = repeat_cast<V, T0, T1, C>;

            using first_tail  = m::sequence<>;
            using second_tail = m::sequence<>;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P>
        struct add_to_cast_sequence_<V, dimension_expr::token<T0, C>, dimension_expr::token<T1, C + P>>
        {
            using type = repeat_cast<V, T0, T1, C>;

            using first_tail  = m::sequence<>;
            using second_tail = m::sequence<dimension_expr::token<T1, P>>;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P>
        struct add_to_cast_sequence_<V, dimension_expr::token<T0, C + P>, dimension_expr::token<T1, C>>
        {
            using type = repeat_cast<V, T0, T1, C>;

            using first_tail  = m::sequence<dimension_expr::token<T1, P>>;
            using second_tail = m::sequence<>;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P>
        struct add_to_cast_sequence_<V, dimension_expr::token<T0, -C>, dimension_expr::token<T1, -C - P>>
        {
            using type = repeat_cast<V, T0, T1, -C>;

            using first_tail  = m::sequence<>;
            using second_tail = m::sequence<dimension_expr::token<T1, -P>>;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P>
        struct add_to_cast_sequence_<V, dimension_expr::token<T0, -C - P>, dimension_expr::token<T1, -C>>
        {
            using type = repeat_cast<V, T0, T1, -C>;

            using first_tail  = m::sequence<dimension_expr::token<T1, -P>>;
            using second_tail = m::sequence<>;
        };

        template <typename, typename, typename>
        struct pos_cast_sequence_;

        template <typename V, typename S0, typename S1>
        using pos_cast_sequence = typename pos_cast_sequence_<V, S0, S1>::type;
        
        template <typename V>
        struct pos_cast_sequence_
        <
            V,
            m::sequence<>,
            m::sequence<>
        >
        {
            using type = m::sequence<>;
        };

        template <typename V, typename T0, typename T1, unsigned C>
        struct pos_cast_sequence_
        <
            V,
            m::sequence<dimension_expr::token<T0, C>>,
            m::sequence<dimension_expr::token<T1, C>>
        >
        {
            using type = repeat_cast<V, T0, T1, C>;
        };

        template <typename V, typename T0, typename T1, typename ...Args0, typename ...Args1>
        struct pos_cast_sequence_<V, m::sequence<T0, Args0...>, m::sequence<T1, Args1...>>
        {
            using type =
            m::concatinate
            <
                typename add_to_cast_sequence_<V, T0, T1>::type,
                pos_cast_sequence
                <
                    V,
                    m::sequence<typename add_to_cast_sequence_<V, T0, T1>::first_tail, Args0...>,
                    m::sequence<typename add_to_cast_sequence_<V, T0, T1>::second_tail, Args1...>
                >
            >;
        };

        /////////

        template <typename, typename, typename>
        struct neg_cast_sequence_;

        template <typename V, typename S0, typename S1>
        using neg_cast_sequence = typename neg_cast_sequence_<V, S0, S1>::type;

        template <typename V>
        struct neg_cast_sequence_
        <
            V,
            m::sequence<>,
            m::sequence<>
        >
        {
            using type = m::sequence<>;
        };

        template <typename V, typename T0, typename T1, unsigned C>
        struct neg_cast_sequence_
        <
            V,
            m::sequence<dimension_expr::token<T0, -C>>,
            m::sequence<dimension_expr::token<T1, -C>>
        >
        {
            using type = repeat_cast<V, T0, T1, -C>;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P, typename ...Args0, typename ...Args1>
        struct neg_cast_sequence_
        <
            V,
            m::sequence<dimension_expr::token<T0, -C>, Args0...>,
            m::sequence<dimension_expr::token<T1, -C - P>, Args1...>
        >
        {
            using type =
            m::concatinate
            <
                repeat_cast<V, T0, T1, -C>,
                neg_cast_sequence
                <
                V,
                m::sequence<Args0...>,
                m::sequence<dimension_expr::token<T1, -P>, Args1...>
                >
            >;
        };

        template <typename V, typename T0, typename T1, unsigned C, unsigned P, typename ...Args0, typename ...Args1>
        struct neg_cast_sequence_
        <
            V,
            m::sequence<dimension_expr::token<T0, -C - P>, Args0...>,
            m::sequence<dimension_expr::token<T1, -C>, Args1...>
        >
        {
            using type =
            m::concatinate
            <
                repeat_cast<V, T0, T1, -C>,
                neg_cast_sequence
                <
                V,
                m::sequence<dimension_expr::token<T1, -P>, Args0...>,
                m::sequence<Args1...>
                >
            >;
        };

        //template <typename V, typename S0, typename S1>
        //struct cast_sequence = m::concatinate<neg_cast_sequence>;
        
        //////////////////////////;
    }

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
