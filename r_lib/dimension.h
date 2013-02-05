#ifndef __DIMENSION_H__
#define __DIMENSION_H__

#include "data_types.h"
#include <iostream>

namespace robot
{

///////////////////////////////////////////////////////////
//
//                  Dimension system
//
///////////////////////////////////////////////////////////

// basic unit

template <int8_t Pow, int8_t Exp>
struct basic_unit
{
    static constexpr int8_t pow = Pow;
    static constexpr int8_t exp = Exp;
};

// pow result

template <int, typename>
struct pow_result_;

template <int P, typename U1>
using pow_result = typename pow_result_<P, U1>::type;

template <int P, int8_t Pow, int8_t Exp>
struct pow_result_<P, basic_unit<Pow, Exp>>
{
    using type = basic_unit<(P * Pow), Exp>;
};

template <int P, typename ...T>
struct pow_result_<P, sequence<T...>>
{
    using type = sequence<pow_result<P, T>...>;
};

// exp result

template <int, typename>
struct exp_result_;

template <int P, typename U1>
using exp_result = typename exp_result_<P, U1>::type;

template <int P, int8_t Pow, int8_t Exp>
struct exp_result_<P, basic_unit<Pow, Exp>>
{
    using type = basic_unit<Pow, (Pow != 0) ? (P + Exp) : 0>;
};

template <int P, typename ...T>
struct exp_result_<P, sequence<T...>>
{
    using type = sequence<exp_result<P, T>...>;
};

// mul result

template <typename, typename>
struct mul_result_;

template <typename U0, typename U1>
using mul_result = typename mul_result_<U0, U1>::type;

template <int8_t Pow0, int8_t Pow1, int8_t Exp0, int8_t Exp1>
struct mul_result_<basic_unit<Pow0, Exp0>, basic_unit<Pow1, Exp1>>
{
    static_assert
    (
        (Exp0 == Exp1 || Pow0 == 0 || Pow1 == 0),
        "incorrect operands"
    );
    using type = basic_unit<Pow0 + Pow1, ((Pow0 == 0) ? Exp1 : Exp0)>;
};

template <typename ...T0, typename ...T1>
struct mul_result_<sequence<T0...>, sequence<T1...>>
{
    using type = sequence<mul_result<T0, T1>...>;
};

// basic unit decimical conversion
// val * 10 ^ Exp1 => val * 10 ^ Exp0

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 > Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v / pow(10, Exp0 - Exp1);
}

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 < Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v * pow(10, Exp1 - Exp0);
}

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 == Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v;
}

template <typename, typename, typename>
struct unit_convertor;

template <typename ValueType>
struct unit_convertor<ValueType, sequence<>, sequence<>>
{
    static ValueType convert(const ValueType& v) { return v; }
};

template
<
    typename ValueType,
    typename Head0, typename ...Tail0,
    typename Head1, typename ...Tail1
>
struct unit_convertor
<
    ValueType,
    sequence<Head0, Tail0...>, 
    sequence<Head1, Tail1...>
>
{
    static_assert
    (
        Head0::pow == Head1::pow,
        "incorrect phisical unit conversion"
    );

    static ValueType convert(const ValueType& v)
    {
        auto tmp = v;

        constexpr auto pow = Head0::pow;
        constexpr auto m = (pow >= 0) ? 1 : -1;
        constexpr auto exp0 = Head0::exp * m;
        constexpr auto exp1 = Head1::exp * m;

        if(pow >= 0)
            for(int8_t i = 0; i < pow; ++i)
                tmp = exp_convert<ValueType, exp0, exp1>(tmp);
        else
            for(int8_t i = 0; i > pow; --i)
                tmp = exp_convert<ValueType, exp0, exp1>(tmp);

        return
        unit_convertor
        <
            ValueType,
            sequence<Tail0...>,
            sequence<Tail1...>
        >::convert(tmp);
    }
};

// phis value

template <typename ValueType, typename Unit>
class phis_value
{
    ValueType value;
public:
    phis_value() : value(0) {}
    phis_value(const ValueType& v) : value(v) {}

    ValueType get_value() const { return value; }
    void set_value(const ValueType& v) { value = v; }
};

// phis_value value type

template <typename T>
struct value_type_
{
    using type = T;
};

template <typename V, typename U>
struct value_type_<phis_value<V, U>>
{
    using type = V;
};

template <typename T>
using value_type = typename value_type_<T>::type;

// phis_value unit type

template <typename> struct unit_type_;

template <typename V, typename U>
struct unit_type_<phis_value<V, U>>
{
    using type = U;
};

template <typename T>
using unit_type = typename unit_type_<T>::type;

// phis_value exp

template <int, typename>
struct dec_factor_;

template <int Exp, typename T>
using dec_factor = typename dec_factor_<Exp, T>::type;

template <int Exp, typename ValueType, typename Unit>
struct dec_factor_<Exp, phis_value<ValueType, Unit>>
{
    using type = phis_value<ValueType, exp_result<Exp, Unit>>;
};

// phis value cast

template <typename T0, typename T1>
T0 phis_cast(const T1& p)
{
    using v_0 = value_type<T0>;
    using u_0 = unit_type<T0>;
    using u_1 = unit_type<T1>;

    return T0(unit_convertor<v_0, u_0, u_1>::convert(p.get_value()));
}

// operators

#define DEF_ARITH_OPERATOR(OP, RET, U0, U1)\
inline phis_value<V, RET> \
operator OP (const phis_value<V, U0>& p0, const phis_value<V, U1>& p1)\
{\
    return phis_value<V, RET>(p0.get_value() OP p1.get_value());\
}

#define MUL(U0, U1) mul_result<U0, U1>
#define DIV(U0, U1) mul_result<U0, pow_result<-1, U1>>

template <typename V, typename U>
DEF_ARITH_OPERATOR(+, U, U, U)

template <typename V, typename U>
DEF_ARITH_OPERATOR(-, U, U, U)

template <typename V, typename U0, typename U1>
DEF_ARITH_OPERATOR(*, MUL(U0, U1), U0, U1)

template <typename V, typename U0, typename U1>
DEF_ARITH_OPERATOR(/, DIV(U0, U1), U0, U1)

#undef DIV
#undef MUL
#undef DEF_ARITH_OPERATOR

#define DEF_COMPRASION_OPERATOR(OP)\
template <typename V0, typename U0, typename V1, typename U1>\
inline \
bool operator OP (const phis_value<V0, U0>& p0, const phis_value<V1, U1>& p1)\
{\
    return (p0.get_value() OP (phis_cast<phis_value<V0, U0>>(p1)).get_value());\
}

DEF_COMPRASION_OPERATOR(==)
DEF_COMPRASION_OPERATOR(!=)
DEF_COMPRASION_OPERATOR(<=)
DEF_COMPRASION_OPERATOR(>=)
DEF_COMPRASION_OPERATOR(< )
DEF_COMPRASION_OPERATOR(> )

#undef DEF_COMPRASION_OPERATOR

// units

namespace basic_units {

template <int8_t ...P>
using unit = sequence<basic_unit<P, 0>...>;

using metre           = unit<1, 0, 0, 0, 0, 0, 0>;
using degree          = unit<0, 1, 0, 0, 0, 0, 0>;
using kilogram        = unit<0, 0, 1, 0, 0, 0, 0>;
using second          = unit<0, 0, 0, 1, 0, 0, 0>;
using kelvin          = unit<0, 0, 0, 0, 1, 0, 0>;
using ampere          = unit<0, 0, 0, 0, 0, 1, 0>;
using volt            = unit<0, 0, 0, 0, 0, 0, 1>;
using non_dimentional = unit<0, 0, 0, 0, 0, 0, 0>;

}

#define DEF_UNIT(NAME)\
template <typename T> using NAME = phis_value<T, basic_units::NAME>;

DEF_UNIT(metre)
DEF_UNIT(degree)
DEF_UNIT(kilogram)
DEF_UNIT(second)
DEF_UNIT(kelvin)
DEF_UNIT(ampere)
DEF_UNIT(volt)
DEF_UNIT(non_dimentional)

#undef DEF_UNIT

///////////////////////////////////////////////////////////
//
//              phis_value serialization
//
///////////////////////////////////////////////////////////

template <typename OStream, typename V, typename U>
OStream& operator << (OStream& os, const phis_value<V, U>& t)
{
    os << t.get_value();
    return os;
}

template <typename IStream, typename V, typename U>
IStream& operator >> (IStream& is, phis_value<V, U>& t)
{
    V v;
    is >> v;
    t.set_value(v);
    return is;
}

}

#endif // __DIMENSION_H__
