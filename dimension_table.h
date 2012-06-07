#ifndef __DIMENSION_TABLE__
#define __DIMENSION_TABLE__

#include <math.h>
#include <stdint.h>

#include "metaprogramming/data_types.h"

namespace robot { namespace dimension
{
    template <int64_t Pow>
    struct decimical_factor
    {
        static constexpr auto value = pow(10, Pow);
    };

    template <typename Quantity, typename Unit, typename Factor> struct dimension;

    template <typename R, typename S>
    struct unit_cast;


struct size;
struct metre;
struct inch;

    template <>
    struct unit_cast<dimension<size, metre, decimical_factor<-2>>, dimension<size, inch, decimical_factor<0>>>
    {
        template <typename T0, typename T1>
        static T0 cast(const T1& t) { return t * 0.0254 / decimical_factor<-2>::value * decimical_factor<0>::value; }
    };

    template <>
    struct unit_cast<double, double>
    {
        template <typename T0, typename T1>
        static T0 cast(const T1& t) { return t * 0.0254 / decimical_factor<-2>::value * decimical_factor<0>::value; }
    };

    template <typename, typename>
    class phis_value;

    template <typename ValueType, typename Quantity, typename Unit, typename Factor>
    class phis_value<ValueType, dimension<Quantity, Unit, Factor>>
    {
        ValueType value;

        using self_unit = dimension<Quantity, Unit, Factor>;

        template <typename U, typename F>
        using src_unit = dimension<Quantity, U, F>;

        template <typename U, typename F>
        using cast_t = unit_cast<self_unit, src_unit<U, F>>;

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

        template <typename T, typename F>
        phis_value& operator=(const phis_value<T, dimension<Quantity, Unit, F>>& p)
        {
            value = p.get() * F::value / Factor::value;
            return *this;
        }

        template <typename T, typename U, typename F>
        phis_value& operator=(const phis_value<T, dimension<Quantity, U, F>>& p)
        {
            value = cast_t<U, F>::cast<int>(p.get());
            return *this;
        }

    };
}}

#endif //__DIMENSION_TABLE__
