#ifndef __SERIALIZATION__
#define __SERIALIZATION__

#include <numeric>

#include "metaprogramming/serialization.h"

namespace robot { namespace package_creation
{

    template <typename ...Args>
    using size = metaprogramming_tools::byte_count<size_t, Args...>;

    namespace serialization
    {
        using namespace metaprogramming_tools;

        template <bool is_const_size, typename T>
        struct size_calc_tmp;

        template <typename T>
        struct size_calc_tmp<true, T>
        {
            constexpr static size_t size(const T& t) { return package_creation::size<T>::value; }
        };

        template <typename T>
        struct size_calc_tmp<false, T>
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            static const bool c = is_const_size<val_t>::value;

            static size_t size(const T& t)
            {
                return
                std::accumulate
                (
                    t.begin(),
                    t.end(),
                    0,
                    [](cref_t p0, cref_t p1) { return p0 + size_calc_tmp<c, val_t>::size(p1); }
                );
            }
        };

        template <typename T>
        constexpr size_t calc_size(const T& t) { return size_calc_tmp<is_const_size<T>::value, T>::size(t); }

        template <bool is_fundamental, typename T>
        struct serializer;

        template <typename T>
        struct serializer<true, T>
        {
            static void serialize(const T& t, uint8_t *pos) { *reinterpret_cast<T*>(pos) = t; }
        };

        template <typename T>
        struct serializer<false, T>
        {
            static void serialize(const T& t, uint8_t *pos)
            {
                using val_t = typename T::value_type;
                using cref_t = typename T::const_reference;

                for(cref_t x : t) {
                    serializer<std::is_fundamental<val_t>::value, val_t>::serialize(x, pos);
                    pos += calc_size(x);
                }
            }
        };

        template <typename T>
        void serialize(const T& t, uint8_t *pos)
        {
            serializer<std::is_fundamental<T>::value, T>::serialize(t, pos);
        }
    }

    namespace package_buffer
    {
        template <typename... Args>
        struct compile_time_size_calc_util
        {   
            constexpr static size_t data_size() { return size<Args...>::value; }
        };

        template <typename ...Args>
        struct const_buffer: public compile_time_size_calc_util<Args...>
        {
            static const uint8_t data[];
        };

        template <typename ...Args>
        struct const_buffer<metaprogramming_tools::sequence<Args...>>: public const_buffer<Args...> {};

        template <typename ...Args>
        const uint8_t const_buffer<Args...>::data[] = { Args::value... };

        template <bool is_const_size, bool is_const_data, typename ...Args>
        struct buffer;

        template <typename... Args>
        struct buffer<false, false, Args...>
        {
            const size_t d_size;
            uint8_t *data;

            buffer(size_t size):
                d_size(size),
                data(new uint8_t[size])
            {}

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
        };

        template <typename... Args>
        struct buffer<true , false, Args...>: public compile_time_size_calc_util<Args...>
        {
            uint8_t data[size<Args...>::value]; 
        };

        template <typename... Args>
        struct buffer<true , true , Args...>: public const_buffer<metaprogramming_tools::serialize<Args...>>
        {};
    }

    template <typename ...Args>
    using package =
    package_buffer::buffer
    <
        metaprogramming_tools::is_const_size<Args...>::value,
        metaprogramming_tools::is_const<Args...>::value,
        Args...
    >;

    namespace package_creation_function
    {
        using namespace metaprogramming_tools;

        template <typename ...Args> struct add_non_const_arg;

        template <typename CurrentList, typename T>
        struct add_non_const_arg<CurrentList, T>
        {
            using type =
            concatinate
            <
                CurrentList,
                at_key
                <
                    is_const<T>,
                    sequence
                    <
                        pair<std::integral_constant<bool, false>, sequence<T>>,
                        pair<std::integral_constant<bool, true >, sequence<>>
                    >
                >
            >;
        };

        template <typename CurrentList, typename T, typename ...Args>
        struct add_non_const_arg<CurrentList, T, Args...>:
            public add_non_const_arg<typename add_non_const_arg<CurrentList, T>::type, Args...>
        {};

        template <typename CurrentList>
        struct add_non_const_arg<CurrentList>
        {
            using type = CurrentList;
        };
        
        template <typename CurrentList, typename ...Args>
        struct add_non_const_arg<CurrentList, sequence<Args...>>:
            public add_non_const_arg<CurrentList, Args...>
        {};

        template <typename CurrentList, typename ...SubSequenceArgs, typename ...Args>
        struct add_non_const_arg<CurrentList, sequence<SubSequenceArgs...>, Args...>:
            public add_non_const_arg<CurrentList, concatinate<sequence<SubSequenceArgs...>, sequence<Args...>>>
        {};

        template <typename ...Args>
        using non_const_args = typename add_non_const_arg<sequence<>, Args...>::type;

        template <typename ...Args>
        using package = package_creation::package<sequence<Args...>>;

        template <typename T0, typename T1, bool const_data, bool const_size>
        struct package_creation_function_util_base;

        template <typename ...Args, typename ...FArgs>
        struct package_creation_function_util_base
        <
            sequence<Args...>,
            sequence<FArgs...>,
            true,
            true
        >
        {
            static package<Args...> make_package(const FArgs&... args)
            {
                return package<Args...>();
            }
        };

        template <typename T0, typename T1, typename P, size_t insert_index>
        struct insert_param;

        template <typename ...Args, typename P, size_t C>
        struct insert_param<sequence<Args...>, sequence<>, P, C>
        {
            static void insert(P&){}
            static void insert(size_t, P&){}

            static size_t size() { return 0; }
        };

        template <typename ...Args, typename ...FArgs, typename F, F U, typename P, size_t C>
        struct insert_param<sequence<std::integral_constant<F, U>, Args...>, sequence<FArgs...>, P, C>
        {
            template <size_t disp>
            using inserter = insert_param<sequence<Args...>, sequence<FArgs...>, P, C + disp>;

            static void insert(P& pack, const FArgs&... args)
            {
                serialization::serialize(U, pack.data + C);
                inserter<serialization::calc_size(U)>::insert(pack, args...);
            }

            static void insert(size_t disp, P& pack, const FArgs&... args)
            {
                serialization::serialize(U, pack.data + disp);
                inserter<0>::insert(disp + serialization::calc_size(U), pack, args...);
            }

            static size_t size(const FArgs&... args)
            {
                return inserter<0>::size(args...) + serialization::calc_size(U);
            }
        };

        template <typename ...Args, typename ...FArgs, typename ...SubSequenceArgs, typename P, size_t C>
        struct insert_param<sequence<sequence<SubSequenceArgs...>, Args...>, sequence<FArgs...>, P, C>
        {
            using inserter =
            insert_param
            <
                concatinate<sequence<SubSequenceArgs...>, sequence<Args...>>,
                sequence<FArgs...>,
                P,
                C
            >;

            static void insert(P& pack, const FArgs&... args)
            {
                inserter::insert(pack, args...);
            }

            static void insert(size_t disp, P& pack, const FArgs&... args)
            {
                inserter::insert(disp, pack, args...);
            }
            
            static size_t size(const FArgs&... args)
            {
                return inserter::size(args...);
            }
        };  

        template <typename ...Args, typename ...FArgs, typename F, typename P, size_t C>
        struct insert_param<sequence<F, Args...>, sequence<F, FArgs...>, P, C>
        {
            template <size_t disp>
            using inserter = insert_param<sequence<Args...>, sequence<FArgs...>, P, C + disp>;

            static void insert(P& pack, const F& f, const FArgs&... args)
            {
                serialization::serialize(f, pack.data + C);
                inserter<serialization::calc_size(f)>::insert(pack, args...);
            }

            static void insert(size_t disp, P& pack, const F& f, const FArgs&... args)
            {
                serialization::serialize(f, pack.data + disp);
                inserter<0>::insert(disp + serialization::calc_size(f), pack, args...);
            }

            static size_t size(const F& f, const FArgs&... args)
            {
                return inserter<0>::size(args...) + serialization::calc_size(f);
            }
        };   

        template <typename ...Args, typename ...FArgs>
        struct package_creation_function_util_base
        <
            sequence<Args...>,
            sequence<FArgs...>,
            false,
            true
        >
        {
            static package<Args...> make_package(const FArgs&... args)
            {
                using pack = package<Args...>;
                pack p;
                insert_param<sequence<Args...>, sequence<FArgs...>, pack, 0>::insert(p, args...);
                return p;
            }
        };

        template <typename ...Args, typename ...FArgs>
        struct package_creation_function_util_base
        <
            sequence<Args...>,
            sequence<FArgs...>,
            false,
            false
        >
        {
            static package<Args...> make_package(const FArgs&... args)
            {
                using pack = package<Args...>;
                using inserter = insert_param<sequence<Args...>, sequence<FArgs...>, pack, 0>;

                pack p(inserter::size(args...));
                inserter::insert(0, p, args...);
                return p;
            }
        };

        template <typename ...Args>
        struct package_creation_function_util: public
            package_creation_function_util_base
            <
                sequence<Args...>,
                non_const_args<Args...>,
                is_const<Args...>::value,
                is_const_size<Args...>::value
            >
        {};

        template <typename ...Args>
        struct package_creation_function_util<sequence<Args...>>: public
            package_creation_function_util<Args...>
        {};
    }

    template <typename T, typename ...Args>
    inline package<T> make_package(const Args&... args)
    {
        return package_creation_function::package_creation_function_util<T>::make_package(args...);
    }
}}

#endif //__SERIALIZATION__
