#ifndef __METAPROGRAMMING_DATA_SEQUENSE__
#define __METAPROGRAMMING_DATA_SEQUENSE__

#include "element_access.h"
#include "type_traits.h"

namespace robot { namespace metaprogramming_tools
{
    template <typename is_const, typename Key, typename T>
    struct data_sequence_element;

    template <typename Key, typename T>
    struct data_sequence_element<std::false_type, Key, T>
    {
        T value;
    };

    template <typename Key, typename T>
    struct data_sequence_element<std::true_type, Key, T>
    {};

    template <typename ...Args>
    struct data_sequence;

    template <>
    struct data_sequence<> {};

    template <typename T, typename ...Args>
    struct data_sequence<T, Args...>:
        public data_sequence_element
               <
                   is_const<T>,
                   typename convert_to_pair<T>::first,
                   typename convert_to_pair<T>::second
               >,
        public data_sequence<Args...>
    {};

    namespace element_access
    {
        template <size_t n, typename ...Args>
        struct at_c<n, data_sequence<Args...>>: public at_c<n, Args...> {};

        template <typename ...Args>
        struct at_c<0, data_sequence<Args...>>: public at_c<0, Args...> {};

        template <typename Key, typename... Args>
        struct at_key<Key, data_sequence<Args...>>: public at_key<Key, Args...> {};

        template <typename ...Args>
        struct data_accessor;

        template <typename Key, typename ...Args>
        struct data_accessor<Key, data_sequence<Args...>>
        {
            using res_t = typename at_key<Key, Args...>::type;
            using element_t = data_sequence_element<is_const<res_t>, Key, res_t>;

            static res_t& get(data_sequence<Args...>& p)
            {
                element_t *r = &p;
                return r->value;
            }
        };
    }

    template <typename Key, typename T>
    at_key<Key, T> get(T& t) { return element_access::data_accessor<Key, T>::get(t); }
}}

#endif //__METAPROGRAMMING_DATA_SEQUENSE__

