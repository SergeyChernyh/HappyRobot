#ifndef __METAPROGRAMMING_SERIALIZATION__
#define __METAPROGRAMMING_SERIALIZATION__

#include <array>
#include <type_traits>
#include <tuple>

#include "element_access.h"
#include "concatination.h"
#include "size.h"

namespace robot { namespace metaprogramming_tools
{
    ///////////////////////////////////////////////////////
    //
    //                      ValueType
    //
    ///////////////////////////////////////////////////////

    namespace get_value_type
    {
        template <typename T> struct value_type { using type = T; };

        template <typename T, T C>
        struct value_type<std::integral_constant<T,C>> { using type = T; };
    }

    template <typename T>
    using value_type = typename get_value_type::value_type<T>::type;

    ///////////////////////////////////////////////////////
    //
    //                  Const Data Check
    //
    ///////////////////////////////////////////////////////

    namespace const_check
    {
        using namespace std;

        template <typename... Args>
        struct is_const;

        template <>
        struct is_const<>: public true_type {};

        template <typename T>
        struct is_const<T>: public false_type {};

        template <typename T, T C>
        struct is_const<integral_constant<T,C>>: public true_type {};

        template <typename T, size_t size>
        struct is_const<array<T, size>>: public is_const<T> {};

        template <typename Head, typename ...Tail>
        struct is_const<Head, Tail...>:
            public
            integral_constant
            <
                bool,
                is_const<Head>::value && is_const<Tail...>::value
            >
        {};

        template <typename ...Args>
        struct is_const<sequence<Args...>>: public is_const<Args...> {};
    }

    template <typename... Args>
    using is_const = std::integral_constant<bool, const_check::is_const<Args...>::value>;

    ///////////////////////////////////////////////////////
    //
    //                  Const Size Check
    //
    ///////////////////////////////////////////////////////

    namespace const_size_check
    {
        using namespace std;

        template <typename... Args>
        struct is_const_size;

        template <>
        struct is_const_size<>: public true_type {};

        template <typename T>
        struct is_const_size<T>: public is_fundamental<value_type<T>> {};

        template <typename T, size_t size>
        struct is_const_size<array<T, size>>: public is_const_size<T> {};
     
        template <typename Head, typename... Tail>
        struct is_const_size<Head, Tail...>:
            public
            integral_constant
            <
                bool,
                is_const_size<Head>::value && is_const_size<Tail...>::value
            >
        {};

        template <typename... Args>
        struct is_const_size<sequence<Args...>>:
            public is_const_size<Args...> {};
        
        template <typename ...Args>
        struct is_const_size<std::tuple<Args...>>:
            public is_const_size<Args...> {};
    }
    
    template <typename... Args>
    using is_const_size =
        std::integral_constant
        <
            bool,
            const_size_check::is_const_size<Args...>::value
        >;

    ///////////////////////////////////////////////////////
    //
    //                  Bytes Count
    //
    ///////////////////////////////////////////////////////

    namespace calc_size
    {
        using namespace std;

        template <typename Size, typename... Args>
        struct size_c;

        template <typename Size>
        struct size_c<Size>: public integral_constant<Size, 0> {};

        // fundamental type

        template <typename Size, typename T>
        struct size_c<Size, T>:
            public
            enable_if
            <
                is_fundamental<value_type<T>>::value,
                integral_constant<Size, sizeof(value_type<T>)>
            >::type
        {};

        // std::array

        template <typename Size, typename T, size_t size>
        struct size_c<Size, array<T, size>>:
            public
            integral_constant
            <
                Size,
                size_c<Size, T>::value * size
            >
        {};

        // type list

        template <typename Size, typename Head, typename... Tail>
        struct size_c<Size, Head, Tail...>:
            public
            integral_constant
            <
                Size,
                size_c<Size, Head>::value + size_c<Size, Tail...>::value
            >
        {};

        template <typename Size, typename... Args>
        struct size_c<Size, sequence<Args...>>:
            public size_c<Size, Args...> {};

        template <typename is_const_size, typename Size, typename ...Args>
        struct size_c_wrap;

        template <typename Size, typename ...Args>
        struct size_c_wrap<std::false_type, Size, Args...>
        {
            using type = unspecified;
        };

        template <typename Size, typename ...Args>
        struct size_c_wrap<std::true_type, Size, Args...>
        {
            using type = std::integral_constant<Size, size_c<Size, Args...>::value>;
        };
    }

    template <typename Size, typename... Args>
    using byte_count = typename calc_size::size_c_wrap<is_const_size<Args...>, Size, Args...>::type;

    ///////////////////////////////////////////////////////
    //
    //               Operations with bytes
    //
    ///////////////////////////////////////////////////////

    template <typename T, size_t n>
    struct tmp_divisor
    {
        static_assert(n < sizeof(T), "robot::metaprogramming_tools::tmp_divisor");

        static const T value = tmp_divisor<T, n - 1>::value * 0x100;
    };

    template <typename T>
    struct tmp_divisor<T, 0> { static const T value = 1; };

    template <typename T, T val, T n>
    using get_byte = std::integral_constant<uint8_t, (val / tmp_divisor<T, n>::value) % 0x100>;

    namespace serialization { namespace bytes
    {
        using namespace std;

        template <size_t n, typename T, T C>
        struct get_bytes
        {
            using type =
            concatinate
            <
                typename get_bytes<n - 1, T, C>::type,
                sequence<integral_constant<uint8_t, get_byte<T, C, n>::value>>
            >;
        };

        template <typename T, T C>
        struct get_bytes<0, T, C>
        {
            using type = sequence<integral_constant<uint8_t, get_byte<T, C, 0>::value>>;
        };
    }}

    template <size_t n, typename T, T C>
    using get_bytes = typename serialization::bytes::get_bytes<n, T, C>::type;

    namespace serialization
    {
        template <typename ...Args>
        struct serialize
        {
            using type = unspecified;
        };

        template <typename T, T C>
        struct serialize<std::integral_constant<T, C>>
        {
            using type = get_bytes<sizeof(T) - 1, T, C>;
        };

        template <typename Head, typename ...Tail>
        struct serialize<Head, Tail...>
        {
            using type =
            concatinate
            <
                typename serialize<Head>::type,
                typename serialize<Tail...>::type
            >;
        };

        template <typename ...Args>
        struct serialize<sequence<Args...>>: public serialize<Args...> {};

        template <>
        struct serialize<sequence<>> { using type = sequence<>; };

        template <typename is_const, typename ...Args>
        struct serialize_wrap;

        template <typename ...Args>
        struct serialize_wrap<std::false_type, Args...>
        {
            using type = unspecified;
        };

        template <typename ...Args>
        struct serialize_wrap<std::true_type, Args...>: public serialize<Args...> {};
    }

    template <typename ...Args>
    using serialize = typename serialization::serialize_wrap<is_const<Args...>, Args...>::type;
}}

#endif // __METAPROGRAMMING_SERIALIZATION__
