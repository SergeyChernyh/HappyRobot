#ifndef __METAPROGRAMMING_SELECT__
#define __METAPROGRAMMING_SELECT__

#include "concatination.h"

namespace robot { namespace utility
{
    namespace selection
    {
        template <template<typename ...P> class F, typename ...Args> struct add_arg;

        template <template<typename ...P> class F, typename S, typename T>
        struct add_arg<F, S, T>
        {
            using type =
            concatinate
            <
                S,
                at_key
                <
                    F<T>,
                    pair<std::false_type, sequence<>>,
                    pair<std::true_type, sequence<T>>
                >
            >;
        };

        template <template<typename ...P> class F, typename S, typename T0, typename T1>
        struct add_arg<F, S, pair<T0, T1>>: public add_arg<F, S, T1> {};

        template <template<typename ...P> class F, typename S, typename T, typename ...Args>
        struct add_arg<F, S, T, Args...>:
            public add_arg<F, typename add_arg<F, S, T>::type, Args...>
        {};

        template <template<typename ...P> class F, typename S>
        struct add_arg<F, S>
        {
            using type = S;
        };
        
        template <template<typename ...P> class F, typename S, typename ...Args>
        struct add_arg<F, S, sequence<Args...>>:
            public add_arg<F, S, Args...>
        {};

        template <template<typename ...P> class F, typename S, typename ...SubArgs, typename ...Args>
        struct add_arg<F, S, sequence<SubArgs...>, Args...>:
            public add_arg<F, S, concatinate<sequence<SubArgs...>, sequence<Args...>>>
        {};
    }

    template <template<typename ...P> class F, typename ...Args>
    using select = typename selection::add_arg<F, sequence<>, Args...>::type;
}}

#endif // __METAPROGRAMMING_SELECT__

