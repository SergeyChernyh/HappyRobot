#ifndef __METAPROGRAMMING_MAP__
#define __METAPROGRAMMING_MAP__

#include "element_access.h"
#include "type_traits.h"

namespace robot { namespace utility
{
    namespace map_details
    {
        template <typename is_const, typename Key, typename T>
        struct map_element;

        template <typename Key, typename T>
        struct map_element<std::false_type, Key, T>
        {
            T value;
        };

        template <typename Key, typename T>
        struct map_element<std::true_type, Key, T>
        {};
    }

    template <typename Key, typename T>
    using map_element = map_details::map_element<is_const<T>, Key, T>;

    template <typename ...Args>
    struct map;

    template <>
    struct map<> {};

    template <typename T, typename ...Args>
    struct map<T, Args...>:
        public map_element
               <
                   typename convert_to_pair<T>::first,
                   typename convert_to_pair<T>::second
               >,
        public map<Args...>
    {};

    namespace element_access
    {
        template <size_t n, typename ...Args>
        struct at_c<n, map<Args...>>: public at_c<n, Args...> {};

        template <typename ...Args>
        struct at_c<0, map<Args...>>: public at_c<0, Args...> {};

        template <typename Key, typename... Args>
        struct at_key<Key, map<Args...>>: public at_key<Key, Args...> {};

        template <typename ...Args>
        struct map_accessor;

        template <typename Key, typename ...Args>
        struct map_accessor<Key, map<Args...>>
        {
            using res_t = typename at_key<Key, Args...>::type;
            using element_t = map_element<Key, res_t>;

            static res_t& get(map<Args...>& p)
            {
                element_t *r = &p;
                return r->value;
            }
        };
    }

    template <typename Key, typename T>
    at_key<Key, T> get(T& t) { return element_access::map_accessor<Key, T>::get(t); }
}}

#endif //__METAPROGRAMMING_MAP__

