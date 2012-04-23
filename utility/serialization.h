#ifndef __METAPROGRAMMING_SERIALIZATION__
#define __METAPROGRAMMING_SERIALIZATION__

#include "type_traits.h"
#include "element_access.h"
#include "concatination.h"
#include "select.h"

namespace robot { namespace utility
{
    ///////////////////////////////////////////////////////
    //
    //           Compile time size calculation
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

        template <typename Size, typename T0, typename T1>
        struct size_c<Size, pair<T0, T1>>: public size_c<Size, T1> {};

        template <typename Size, typename... Args>
        struct size_c<Size, sequence<Args...>>: public size_c<Size, Args...> {};

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
        static_assert(n < sizeof(T), "robot::utility::tmp_divisor");

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

    ///////////////////////////////////////////////////////
    //
    //            Compile time serialization
    //
    ///////////////////////////////////////////////////////

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

        template <typename T0, typename T1>
        struct serialize<pair<T0, T1>>: public serialize<T1> {};

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
            constexpr static size_t size(const T& t) { return byte_count<size_t, T>::value; }
        };

        // stl container serialize

        template <typename T>
        struct size_c_tmp<false, T>
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
                    [](cref_t p0, cref_t p1) { return p0 + size_c_tmp<c, val_t>::size(p1); }
                );
            }
        };

        template <typename T>
        constexpr size_t size_c(const T& t) { return size_c_tmp<is_const_size<T>::value, T>::size(t); }
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
        struct serializer<false, pair<T0, T1>>: public serializer<std::is_fundamental<T1>::value, T1> {};

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
}}

#endif // __METAPROGRAMMING_SERIALIZATION__
