#ifndef __NO_SYSTEM_SI_METRICS__
#define __NO_SYSTEM_SI_METRICS__

#include "phis_value.h"

namespace robot { namespace no_system_si_metrics
{
    namespace quantity
    {
        struct angle;
    }

    namespace unit
    {
        struct degree;
    }

    namespace dimension
    {
        template <int64_t DecPower>
        using factor = dim::decimical_factor<DecPower>;

        template <typename Quantity, typename Unit>
        using dimension = dim::dimension<Quantity, Unit, factor<0>>;

        using degree= dimension<quantity::angle, unit::degree>;
    }

    template <typename T, int64_t DecPower>
    using apply_factor = dim::apply_dec_factor<T, DecPower>;

    template <typename ValueFormat = double>
    using degree= phis_value<ValueFormat, dimension::degree>;
}}

#endif //__NO_SYSTEM_SI_METRICS__
