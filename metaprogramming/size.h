#ifndef __METAPROGRAMMING_SIZE__
#define __METAPROGRAMMING_SIZE__

#include <cstring>
#include "sequence.h"

namespace robot { namespace metaprogramming_tools
{    
    namespace sequence_size
    {
        template <typename ...Args>
        struct size
        {
            using type = std::integral_constant<size_t, sizeof...(Args)>;
        };

        template <typename ...Args>
        struct size<sequence<Args...>>: public size<Args...> {};
    }

    template <typename ...Args>
    using size = typename sequence_size::size<Args...>::type;
}}

#endif // __METAPROGRAMMING_SIZE__
