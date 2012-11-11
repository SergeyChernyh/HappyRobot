#ifndef __TEXT_ASM_PARSER_H__
#define __TEXT_ASM_PARSER_H__

#include <iostream>
#include <vector>
#include "sequence.h"

namespace robot {

struct constant_mismatch_error: public std::logic_error
{
    constant_mismatch_error(): std::logic_error("parser error: const package field mismatch") {}
};

template <typename T, T C>
inline std::istream& operator>>(std::istream& is, const std::integral_constant<T, C>& t)
{
    T tmp;
    is >> tmp;

    if(C != tmp)
        throw constant_mismatch_error();

    return is;
}

inline std::istream& operator>>(std::istream& is, metaprogramming::sequence<>& t)
{
    return is;
}

template <typename T>
using enable_if_const = typename std::enable_if<metaprogramming::is_const<T>::value, std::istream>::type;

template <typename T>
using enable_if_not_const = typename std::enable_if<!metaprogramming::is_const<T>::value, std::istream>::type;

template <typename Head, typename ...Tail>
inline std::istream& operator>>(enable_if_not_const<Head>& is, metaprogramming::sequence<Head, Tail...>& t)
{
    is >> at_c<0>(t);

    metaprogramming::sequence<Tail...>& tail = t;

    is >> tail;

    return is;
}

template <typename Head, typename ...Tail>
inline std::istream& operator>>(enable_if_const<Head>& is, metaprogramming::sequence<Head, Tail...>& t)
{
    is >> Head();

    metaprogramming::sequence<Tail...>& tail = t;

    is >> tail;

    return is;
}

}
#endif
