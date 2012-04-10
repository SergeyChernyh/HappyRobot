#ifndef __METAPROGRAMMING_PAIR__
#define __METAPROGRAMMING_PAIR__

namespace robot { namespace metaprogramming_tools
{
    ///////////////////////////////////////////////////////
    //
    //                Sequense & unspecified
    //
    ///////////////////////////////////////////////////////

    struct unspecified {};

    template <typename ...Args> struct sequence {};

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
    //            Specific type traits: is_const
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
    //         Specific type traits: is_const_size
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
    //                        Pair
    //
    ///////////////////////////////////////////////////////

    template <typename First, typename Second>
    struct pair
    {
        using first  = First;
        using second = Second;
    };

    namespace to_pair
    {
        template <typename ...Args>
        struct convert_to_pair;

        template <typename T>
        struct convert_to_pair<T>
        {
            using type = pair<unspecified, T>;
        };

        template
        <
            template <typename F, typename S> class Pair,
            typename First,
            typename Second
        >
        struct convert_to_pair<Pair<First, Second>>
        {
            using type = pair<First, Second>;
        };
    }
    
    template <typename ...Args>
    using convert_to_pair = typename to_pair::convert_to_pair<Args...>::type;
}}

#endif //__METAPROGRAMMING_PAIR__
