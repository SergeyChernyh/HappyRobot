#ifndef __SYSTEM_SI_METRICS__
#define __SYSTEM_SI_METRICS__

#include "phis_value.h"

namespace robot { namespace system_si_metrics
{
    namespace quantity
    {
        struct length;
        struct mass;
        struct time;
        struct electric_current;
        struct thermodynamic_temperature;
        struct luminous_intensity;
        struct amount_of_substance;
    }

    namespace unit
    {
        struct metre;
        struct kilogram;
        struct second;
        struct ampere;
        struct kelvin;
        struct candela;
        struct mole;
    }

    namespace dimension
    {
        template <int64_t DecPower>
        using factor = dim::decimical_factor<DecPower>;

        template <typename Quantity, typename Unit>
        using dimension = dim::dimension<Quantity, Unit, factor<0>>;

        using metre    = dimension<quantity::length                   , unit::metre   >;
        using kilogram = dimension<quantity::mass                     , unit::kilogram>;
        using second   = dimension<quantity::time                     , unit::second  >;
        using ampere   = dimension<quantity::electric_current         , unit::ampere  >;
        using kelvin   = dimension<quantity::thermodynamic_temperature, unit::kelvin  >;
        using candela  = dimension<quantity::luminous_intensity       , unit::candela >;
        using mole     = dimension<quantity::amount_of_substance      , unit::mole    >;
    }

    template <typename T, int64_t DecPower>
    using apply_factor = dim::apply_dec_factor<T, DecPower>;

    template <typename ValueFormat = double>
    using metre = phis_value<ValueFormat, dimension::metre>;

    template <typename ValueFormat = double>
    using kilogram = phis_value<ValueFormat, dimension::kilogram>;

    template <typename ValueFormat = double>
    using second = phis_value<ValueFormat, dimension::second>;

    template <typename ValueFormat = double>
    using ampere = phis_value<ValueFormat, dimension::ampere>;

    template <typename ValueFormat = double>
    using kelvin = phis_value<ValueFormat, dimension::kelvin>;

    template <typename ValueFormat = double>
    using candela = phis_value<ValueFormat, dimension::candela>;

    template <typename ValueFormat = double>
    using mole = phis_value<ValueFormat, dimension::mole>;
}}

#endif //__SYSTEM_SI_METRICS__
