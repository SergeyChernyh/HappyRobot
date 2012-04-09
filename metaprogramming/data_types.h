#ifndef __METAPROGRAMMING_PAIR__
#define __METAPROGRAMMING_PAIR__

namespace robot { namespace metaprogramming_tools
{
    struct unspecified {};

    template <typename ...Args> struct sequence {};

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
