#ifndef __PACKAGE__
#define __PACKAGE__

#include <numeric>

#include "metaprogramming/serialization.h"
#include "metaprogramming/select.h"
#include "metaprogramming/map.h"

namespace robot { namespace package_creation
{

    ///////////////////////////////////////////////////////
    //
    //           Run time size calculation
    //
    ///////////////////////////////////////////////////////

    namespace run_time_calc_size
    {
        template <bool is_const_size, typename T>
        struct size_c_tmp;

        // const size type serialize

        template <typename T>
        struct size_c_tmp<true, T>
        {
            constexpr static size_t size(const T& t) { return metaprogramming::byte_count<size_t, T>::value; }
        };

        // stl container serialize

        template <typename T>
        struct size_c_tmp<false, T>
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            static const bool c = metaprogramming::is_const_size<val_t>::value;

            static size_t size(const T& t)
            {
                return
                std::accumulate
                (
                    t.begin(),
                    t.end(),
                    0,
                    [](cref_t p0, cref_t p1) { return p0 + size_c_tmp<c, val_t>::size(p1); }
                );
            }
        };

        template <typename T>
        constexpr size_t size_c(const T& t) { return size_c_tmp<metaprogramming::is_const_size<T>::value, T>::size(t); }
    }

    namespace run_time_serialization
    {
        template <bool is_fundamental, typename T>
        struct serializer;

        template <typename T>
        struct serializer<true, T>
        {
            static void   serialize(const T& t,       uint8_t *pos) { *reinterpret_cast<T*>(pos) = t; }
            static void deserialize(      T& t, const uint8_t *pos) { t = *reinterpret_cast<const T*>(pos); }
        };

        template <typename T>
        struct serializer<false, T>
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            static void serialize(const T& t, uint8_t *pos)
            {
                for(cref_t x : t) {
                    serializer<std::is_fundamental<val_t>::value, val_t>::serialize(x, pos);
                    pos += run_time_calc_size::size_c(x);
                }
            }

            static void deserialize(T& t, const uint8_t *pos)
            {
                for(cref_t x : t) {
                    serializer<std::is_fundamental<val_t>::value, val_t>::deserialize(x, pos);
                    pos += run_time_calc_size::size_c(x);
                }
            }
        };

        template <typename T0, typename T1>
        struct serializer<false, metaprogramming::pair<T0, T1>>: public serializer<std::is_fundamental<T1>::value, T1> {};

        template <typename T>
        void serialize(uint8_t *pos, const T& t)
        {
            serializer<std::is_fundamental<T>::value, T>::serialize(t, pos);
        }

        template <typename T>
        void deserialize(const uint8_t *pos, T& t)
        {
            serializer<std::is_fundamental<T>::value, T>::deserialize(t, pos);
        }
    }

    namespace run_time_serialization_utility
    {
        using namespace metaprogramming;

        template <typename T0, typename T1>
        struct tmp_serializer;

        template <>
        struct tmp_serializer<sequence<>, sequence<>>
        {
            static void serialize(uint8_t*){}
            static void deserialize(const uint8_t*){}
            static size_t size() { return 0; }
        };

        template <typename ...Args, typename ...FArgs, typename F, F U>
        struct tmp_serializer<sequence<std::integral_constant<F, U>, Args...>, sequence<FArgs...>>
        {
            using inserter = tmp_serializer<sequence<Args...>, sequence<FArgs...>>;

            static void serialize(uint8_t *dst, const FArgs&... args)
            {
                run_time_serialization::serialize(dst, U);
                inserter::serialize(dst + run_time_calc_size::size_c(U), args...);
            }

            static void deserialize(const uint8_t *dst, FArgs&... args)
            {
                //run_time_serialization::deserialize(dst, U); TODO add check
                inserter::deserialize(dst + run_time_calc_size::size_c(U), args...);
            }

            static size_t size(const FArgs&... args)
            {
                return inserter::size(args...) + run_time_calc_size::size_c(U);
            }
        };

        template <typename ...Args, typename ...FArgs, typename F>
        struct tmp_serializer<sequence<F, Args...>, sequence<F, FArgs...>>
        {
            using inserter = tmp_serializer<sequence<Args...>, sequence<FArgs...>>;

            static void serialize(uint8_t *dst, const F& f, const FArgs&... args)
            {
                run_time_serialization::serialize(dst, f);
                inserter::serialize(dst + run_time_calc_size::size_c(f), args...);
            }

            static void deserialize(const uint8_t *dst, F& f, FArgs&... args)
            {
                run_time_serialization::deserialize(dst, f);
                inserter::deserialize(dst + run_time_calc_size::size_c(f), args...);
            }

            static size_t size(const F& f, const FArgs&... args)
            {
                return inserter::size(args...) + run_time_calc_size::size_c(f);
            }
        };

        template <typename ...Args, typename ...FArgs, typename T0, typename T1>
        struct tmp_serializer<sequence<pair<T0, T1>, Args...>, sequence<FArgs...>>:
            public tmp_serializer<sequence<T1, Args...>, sequence<FArgs...>> {};

        template <typename ...Args, typename ...FArgs, typename ...SubSequenceArgs>
        struct tmp_serializer<sequence<sequence<SubSequenceArgs...>, Args...>, sequence<FArgs...>>:
            public tmp_serializer<sequence<SubSequenceArgs..., Args...>, sequence<FArgs...>> {};

        template <typename ...Args>
        struct serializer: public tmp_serializer<Args..., select<is_no_const, Args...>> {};
    }

    namespace run_time_calc_size
    {
        template <typename ...Args, typename ...Fargs>
        size_t size_c(const Fargs&... args)
        {
            return run_time_serialization_utility::serializer<Args...>::size(args...);
        }
    }

    namespace run_time_serialization
    {
        template <typename ...Args, typename ...Fargs>
        void serialize(uint8_t *pos, const Fargs&... args)
        {
            run_time_serialization_utility::serializer<Args...>::serialize(pos, args...);
        }

        template <typename ...Args, typename ...Fargs>
        void deserialize(const uint8_t *pos, Fargs&... args)
        {
            run_time_serialization_utility::serializer<Args...>::deserialize(pos, args...);
        }
    }

    template <typename ...Args>
    using no_const_args = metaprogramming::select<metaprogramming::is_no_const, Args...>;

    template <typename ...Args>
    using size = metaprogramming::byte_count<size_t, Args...>;

    template <typename ...Args>
    using pattern = metaprogramming::sequence<Args...>;

    namespace package_buffer
    {
        namespace calc_size = run_time_calc_size;
        namespace serialization = run_time_serialization;

        template <typename... Args>
        struct compile_time_size_calc_util
        {   
            constexpr static size_t data_size() { return size<Args...>::value; }
        };

        template <typename ...Args>
        class const_buffer: public compile_time_size_calc_util<Args...>
        {
            static const uint8_t data[];

        public:
            const uint8_t* get_data() const { return data; }
        };

        template <typename ...Args>
        struct const_buffer<pattern<Args...>>: public const_buffer<Args...> {};

        template <typename ...Args>
        const uint8_t const_buffer<Args...>::data[] = { Args::value... };

        template <bool is_const_size, typename NonConstArgs, typename ...Args>
        class buffer;

        template <typename... Args, typename ...NonConstArgs>
        class buffer<false, pattern<NonConstArgs...>, Args...>
        {
            const size_t d_size;
            uint8_t *data;

        public:
            buffer(const NonConstArgs&... args):
                d_size(calc_size::size_c<Args...>(args...)),
                data(new uint8_t[d_size])
            {
                serialization::serialize<Args...>(data, args...);
            }

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args, typename ...NonConstArgs>
        class buffer<true, pattern<NonConstArgs...>, Args...>: public compile_time_size_calc_util<Args...>
        {
            uint8_t data[size<Args...>::value];

        public:
            buffer(const NonConstArgs&... args)
            {
                serialization::serialize<Args...>(data, args...);
            }

            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args>
        class buffer<true, pattern<>, Args...>: public const_buffer<metaprogramming::serialize<Args...>>
        {};
    }

    template <typename ...Args>
    using package =
    package_buffer::buffer
    <
        metaprogramming::is_const_size<Args...>::value,
        no_const_args<Args...>,
        Args...
    >;
}}

#endif //__PACKAGE__
