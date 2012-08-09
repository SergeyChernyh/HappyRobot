#ifndef __DIMENSION_TABLE__
#define __DIMENSION_TABLE__

#include <math.h>
#include <stdint.h>

#include <type_traits> 

#include "metaprogramming/select.h"

namespace robot { namespace dim
{
    namespace dimension_expr
    {
        namespace m = metaprogramming;

        template <typename, int>
        struct token{};

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

    namespace cast_details
    {
        namespace m = metaprogramming;

        template <typename R, typename U0, typename U1>
        struct dimension_cast;

        template <typename R, typename Q, typename U0, typename U1, typename F0, typename F1>
        struct dimension_cast<R, dimension<Q, U0, F0>, dimension<Q, U1, F1>>
        {
            template <typename T>
            static R cast(const T& t) { return unit_cast<R, U0, U1>::cast(t) * F0::value / F1::value; }
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
            template <typename T>
            static R cast(const T& t)
            {
                return repeat_cast_<R, T0, T1, C - 1, 1>::cast(dimension_cast<R, T0, T1>::cast(t));
            }
        };

        template <typename R, typename T0, typename T1>
        struct repeat_cast_<R, T0, T1, 1, 1>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return dimension_cast<R, T0, T1>::cast(t);
            }
        };

        template <typename R, typename T0, typename T1, int C>
        struct repeat_cast_<R, T0, T1, C, 0>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return repeat_cast_<R, T0, T1, C + 1, 0>::cast(reverse_cast<R, T0, T1>::cast(t));
            }
        };

        template <typename R, typename T0, typename T1>
        struct repeat_cast_<R, T0, T1, -1, 0>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return reverse_cast<R, T0, T1>::cast(t);
            }
        };

        template <typename, typename, typename>
        struct token_cast_;

        template <typename R, typename T0, typename T1, int C>
        struct token_cast_<R, dimension_expr::token<T0, C>, dimension_expr::token<T1, C>>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return repeat_cast_<R, T0, T1, C, (C > 0)>::cast(t);
            }

            using first_tail = m::sequence<>;
            using second_tail = m::sequence<>;
        };

        template <typename T, int C>
        struct token_cast_tail_
        {
            using type = m::sequence<dimension_expr::token<T, C>>;
        };

        template <typename T>
        struct token_cast_tail_<T, 0>
        {
            using type = m::sequence<>;
        };

        template <typename R, typename T0, typename T1, int C0, int C1>
        struct token_cast_<R, dimension_expr::token<T0, C0>, dimension_expr::token<T1, C1>>
        {
            static_assert((C0 > 0 && C1 > 0) || (C0 < 0 && C1 < 0), "token_cast_ error: sign mismatch");

            static constexpr int min() { return C0 > 0 ? C0 < C1 ? C0 : C1 : C0 < C1 ? C1 : C0; }

            static constexpr int sub(int p) { return C0 > 0 ? p - min() : p + min(); }

            template <typename T>
            static R cast(const T& t)
            {
                return repeat_cast_<R, T0, T1, min(), (min() > 0)>::cast(t);
            }

            using first_tail  = typename token_cast_tail_<T0, sub(C0)>::type;
            using second_tail = typename token_cast_tail_<T1, sub(C1)>::type;
        };

        template <typename, typename, typename>
        struct sequence_cast;

        template <typename R, typename T0, typename T1, typename ...Args0, typename ...Args1>
        struct sequence_cast<R, m::sequence<T0, Args0...>, m::sequence<T1, Args1...>>
        {
            template <typename T>
            static R cast(const T& t)
            {
                using t_cast = token_cast_<R, T0, T1>;
                return t_cast::cast(t);
                sequence_cast
                <
                    R,
                    m::concatinate<typename t_cast::first_tail , m::sequence<Args0...>>,
                    m::concatinate<typename t_cast::second_tail, m::sequence<Args1...>>
                >::cast(t_cast::cast(t));
            }
        };

        template <typename R>
        struct sequence_cast<R, m::sequence<>, m::sequence<>>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return t;
            }
        };

        //////////////////////////;
        
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

        template <typename T>
        struct q_expr_
        {
            using type = quantity<T>;
        };

        template <typename T>
        using q_expr = typename q_expr_<T>::type;

        template <typename ...Args>
        struct q_expr_<m::sequence<Args...>>
        {
            using type = m::sequence<q_expr<Args>...>;
        };

        template <typename>
        struct pos_pow;

        template <typename T, int C>
        struct pos_pow<dimension_expr::token<T, C>>
        {
            static const bool value = (C > 0);
        };

        template <typename, typename, typename>
        struct full_cast;

        template <typename R, typename Head, typename ...Args0, typename ...Args1>
        struct full_cast<R, m::sequence<Head, Args0...>, m::sequence<Args1...>>
        {
            template <typename A>
            using equal = std::integral_constant<bool, std::is_same<quantity<Head>, quantity<A>>::value>;

            template <typename A>
            using pos_equal = std::integral_constant<bool, equal<A>::value && pos_pow<A>::value>;

            template <typename A>
            using neg_equal = std::integral_constant<bool, equal<A>::value && !pos_pow<A>::value>;

            template <typename A>
            using not_equal = std::integral_constant<bool, !std::is_same<quantity<Head>, quantity<A>>::value>;

            template <typename T>
            static R cast(const T& t)
            {
                using seq0 = m::sequence<Head, Args0...>;
                using seq1 = m::sequence<Args1...>;

                using head_pos_0 = m::select<pos_equal, seq0>;
                using head_pos_1 = m::select<pos_equal, seq1>;

                using head_neg_0 = m::select<neg_equal, seq0>;
                using head_neg_1 = m::select<neg_equal, seq1>;

                using tail_0 = m::select<not_equal, seq0>;
                using tail_1 = m::select<not_equal, seq1>;

                using pos_cast = sequence_cast<R, head_pos_0, head_pos_1>;
                using neg_cast = sequence_cast<R, head_neg_0, head_neg_1>;

                return pos_cast::cast(neg_cast::cast(full_cast<R, tail_0, tail_1>::cast(t)));
            }
        };

        template <typename R>
        struct full_cast<R, m::sequence<>, m::sequence<>>
        {
            template <typename T>
            static R cast(const T& t)
            {
                return t;
            }
        };
    }

    template <typename ValueType, typename Dimension>
    class phis_value_
    {
        ValueType value;

    public:
        phis_value_() {}
        phis_value_(const ValueType& v): value(v) {}

        void set(const ValueType& v) { value = v; }
        ValueType get() const { return value; }

        operator ValueType() const { return value; }

        phis_value_& operator=(const phis_value_& p)
        {
            value = p.value;
            return *this;
        }

        template <typename V>
        phis_value_& operator=(const phis_value_<V, Dimension>& p)
        {
            value = p.value;
            return *this;
        }

        template <typename V, typename D>
        phis_value_& operator=(const phis_value_<V, D>& p)
        {
            static_assert(is_equal<cast_details::q_expr<Dimension>, cast_details::q_expr<D>>::value, "phis value assignment error: quontity mismatch");
            value = cast_details::full_cast<ValueType, expr<D>, expr<Dimension>>::cast(p.get());
            return *this;
        }
    };

    template <int64_t Pow>
    struct decimical_factor
    {
        static constexpr auto value = pow(10, Pow);
    };

    template <typename T, int64_t DecPow>
    struct apply_dec_factor_;

    template <typename T, int64_t DecPow>
    using apply_dec_factor = typename apply_dec_factor_<T, DecPow>::type;

    template <int64_t DecPow0, int64_t DecPow1>
    struct apply_dec_factor_<decimical_factor<DecPow0>, DecPow1>
    {
        using type = decimical_factor<DecPow0 + DecPow1>;
    };

    template <typename Q, typename U, typename F, int64_t DecPow>
    struct apply_dec_factor_<dimension<Q, U, F>, DecPow>
    {
        using type = dimension<Q, U, apply_dec_factor<F, DecPow>>;
    };

    template <typename V, typename D, int64_t DecPow>
    struct apply_dec_factor_<phis_value_<V, D>, DecPow>
    {
        using type = phis_value_<V, apply_dec_factor<D, DecPow>>;
    };
}}

namespace robot
{
    template <typename ValueType, typename Dimension>
    using phis_value = dim::phis_value_<ValueType, Dimension>;

    template <typename V0, typename D0, typename V1, typename D1>
    inline
    phis_value<decltype(V0() * V1()), dim::expr<D0, D1>>
    operator *(const phis_value<V0, D0>& p0, const phis_value<V1, D1>& p1)
    {
        return phis_value<decltype(V0() * V1()), dim::expr<D0, D1>>(p0.get() * p1.get());
    }

    template <typename V0, typename D0, typename V1, typename D1>
    inline
    phis_value<decltype(V0() / V1()), dim::expr<D0,  dim::power<D1, -1>>>
    operator /(const phis_value<V0, D0>& p0, const phis_value<V1, D1>& p1)
    {
        return phis_value<decltype(V0() / V1()), dim::expr<D0,  dim::power<D1, -1>>>(p0.get() / p1.get());
    }

    template <typename V0, typename D0, typename V1, typename D1>
    inline
    phis_value<decltype(V0() + V1()), D0>
    operator +(const phis_value<V0, D0>& p0, const phis_value<V1, D1>& p1)
    {
        return phis_value<decltype(V0() + V1()), D0>(p0.get() + p1.get());
    }

    template <typename V0, typename D0, typename V1, typename D1>
    inline
    phis_value<decltype(V0() - V1()), D0>
    operator -(const phis_value<V0, D0>& p0, const phis_value<V1, D1>& p1)
    {
        return phis_value<decltype(V0() - V1()), D0>(p0.get() - p1.get());
    }
}

#endif //__DIMENSION_TABLE__
