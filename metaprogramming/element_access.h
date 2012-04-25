#ifndef __METAPROGRAMMING_ELEMENT_ACCESS__
#define __METAPROGRAMMING_ELEMENT_ACCESS__

#include <cstring>
#include <stdint.h>

#include "data_types.h"

namespace robot { namespace metaprogramming
{
    // by index ///////////////////////////////////////////

    namespace element_access
    {
        template <size_t n, typename ...Args> struct at_c;
     
        template <typename Head, typename ...Tail>
        struct at_c<0, Head, Tail...>
        {
            using type = Head;
        };
         
        template <size_t n, typename Head, typename ...Tail>
        struct at_c<n, Head, Tail...>: public at_c<n - 1, Tail...>
        {
            static_assert
            (
                n <= sizeof...(Tail),
                "robot::metaprogramming::at: out of range"
            );
        };

        template <size_t n, typename ...Args>
        struct at_c<n, sequence<Args...>>: public at_c<n, Args...> {};

        template <typename ...Args>
        struct at_c<0, sequence<Args...>>: public at_c<0, Args...> {};
    }

    template <size_t n, typename ...Args>
    using at_c = typename element_access::at_c<n, Args...>::type;

    template <typename T, typename ...Args>
    using at = typename element_access::at_c<T::value, Args...>::type;

    // by key /////////////////////////////////////////////

    namespace element_access
    {
        template <typename Key, typename ...Args> struct at_key;

        template <typename Head, typename ...Tail>
        struct at_key<typename convert_to_pair<Head>::first, Head, Tail...>
        {
            using type = typename convert_to_pair<Head>::second;
        };

        template <typename Key, typename Head, typename ...Tail>
        struct at_key<Key, Head, Tail...>: public at_key<Key, Tail...> {};

        template <typename Key, typename... Args>
        struct at_key<Key, sequence<Args...>>: public at_key<Key, Args...> {};
    }

    template <typename Key, typename ...Args>
    using at_key = typename element_access::at_key<Key, Args...>::type;
}}

#endif // __METAPROGRAMMING_ELEMENT_ACCESS__
