#ifndef __SERIALIZATION__
#define __SERIALIZATION__

#include <numeric>

#include "metaprogramming/serialization.h"
#include "metaprogramming/select.h"

namespace robot { namespace package_creation
{

    template <typename ...Args>
    using size = metaprogramming_tools::byte_count<size_t, Args...>;

    namespace serialization
    {
        using namespace metaprogramming_tools;

        ///////////////////////////////////////////////////
        //
        //              Size calculation
        //
        ///////////////////////////////////////////////////

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

        ///////////////////////////////////////////////////
        //
        // Fundamental types and containers serialization
        //
        ///////////////////////////////////////////////////

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

        template <typename T0, typename T1>
        struct serializer<false, pair<T0, T1>>: public serializer<std::is_fundamental<T1>::value, T1> {};

        template <typename T>
        void serialize(const T& t, uint8_t *pos)
        {
            serializer<std::is_fundamental<T>::value, T>::serialize(t, pos);
        }

        ///////////////////////////////////////////////////
        //
        //            Sequences serialization
        //
        ///////////////////////////////////////////////////

        // T0 - sequence<all_types...>
        // T1 - sequence<only_non_const_types...>

        template <typename T0, typename T1>
        struct insert_param;

        template <>
        struct insert_param<sequence<>, sequence<>>
        {
            static void insert(uint8_t*){}
            static size_t size() { return 0; }
        };

        template <typename ...Args, typename ...FArgs, typename F, F U>
        struct insert_param<sequence<std::integral_constant<F, U>, Args...>, sequence<FArgs...>>
        {
            using inserter = insert_param<sequence<Args...>, sequence<FArgs...>>;

            static void insert(uint8_t *dst, const FArgs&... args)
            {
                serialize(U, dst);
                inserter::insert(dst + calc_size(U), args...);
            }

            static size_t size(const FArgs&... args)
            {
                return inserter::size(args...) + calc_size(U);
            }
        };

        template <typename ...Args, typename ...FArgs, typename F>
        struct insert_param<sequence<F, Args...>, sequence<F, FArgs...>>
        {
            using inserter = insert_param<sequence<Args...>, sequence<FArgs...>>;

            static void insert(uint8_t *dst, const F& f, const FArgs&... args)
            {
                serialize(f, dst);
                inserter::insert(dst + calc_size(f), args...);
            }

            static size_t size(const F& f, const FArgs&... args)
            {
                return inserter::size(args...) + calc_size(f);
            }
        };

        template <typename ...Args, typename ...FArgs, typename T0, typename T1>
        struct insert_param<sequence<pair<T0, T1>, Args...>, sequence<FArgs...>>:
            public insert_param<sequence<T1, Args...>, sequence<FArgs...>> {};

        template <typename ...Args, typename ...FArgs, typename ...SubSequenceArgs>
        struct insert_param<sequence<sequence<SubSequenceArgs...>, Args...>, sequence<FArgs...>>:
            public insert_param<sequence<SubSequenceArgs..., Args...>, sequence<FArgs...>> {};
    }

    namespace package_buffer
    {
        namespace m = metaprogramming_tools;

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
        struct const_buffer<m::sequence<Args...>>: public const_buffer<Args...> {};

        template <typename ...Args>
        const uint8_t const_buffer<Args...>::data[] = { Args::value... };

        template <bool is_const_size, typename NonConstArgs, typename ...Args>
        struct buffer;

        template <typename... Args, typename ...NonConstArgs>
        struct buffer<false, m::sequence<NonConstArgs...>, Args...>
        {
            using inserter = serialization::insert_param<m::sequence<Args...>, m::sequence<NonConstArgs...>>;

            const size_t d_size;
            uint8_t *data;

            buffer(const NonConstArgs&... args):
                d_size(inserter::size(args...)),
                data(new uint8_t[d_size])
            {
                inserter::insert(data, args...);
            }

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
        };

        template <typename... Args, typename ...NonConstArgs>
        struct buffer<true, m::sequence<NonConstArgs...>, Args...>: public compile_time_size_calc_util<Args...>
        {
            using inserter = serialization::insert_param<m::sequence<Args...>, m::sequence<NonConstArgs...>>;
            uint8_t data[size<Args...>::value];

            buffer(const NonConstArgs&... args)
            {
                inserter::insert(data, args...);
            }
        };

        template <typename... Args>
        struct buffer<true, m::sequence<>, Args...>: public const_buffer<m::serialize<Args...>>
        {};
    }

    template <typename ...Args>
    using package =
    package_buffer::buffer
    <
        metaprogramming_tools::is_const_size<Args...>::value,
        metaprogramming_tools::select<metaprogramming_tools::is_no_const, Args...>,
        Args...
    >;
}}

#endif //__SERIALIZATION__
