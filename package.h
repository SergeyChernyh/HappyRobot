#ifndef __PACKAGE__
#define __PACKAGE__

#include <numeric>

#include "metaprogramming/serialization.h"
#include "metaprogramming/select.h"
#include "metaprogramming/map.h"

namespace robot { namespace package_creation
{
    template <typename ...Args>
    using no_const_args = metaprogramming::select<metaprogramming::is_no_const, Args...>;

    template <typename ...Args>
    using size = metaprogramming::byte_count<size_t, Args...>;

    template <typename ...Args>
    using pattern = metaprogramming::sequence<Args...>;

    namespace serialization
    {
        template <typename T0, typename T1>
        using pair = metaprogramming::pair<T0, T1>;

        template <bool, typename>
        class size_c_tmp;

        template <bool, typename>
        class serialize_tmp;

        // const size type serialize

        template <typename T>
        class size_c_tmp<true, T>
        {
        public:
            constexpr static size_t size(const T& t) { return robot::package_creation::size<T>::value; }
        };

        // stl container serialize

        template <typename T>
        class size_c_tmp<false, T>
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            static const bool c = metaprogramming::is_const_size<val_t>::value;

        public:
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
        class size_c_tmp_wrapper:
            public size_c_tmp<metaprogramming::is_const_size<T>::value, T>
        {};

        template <typename T0, typename T1>
        class size_c_tmp_wrapper<pair<T0, T1>>:
            public size_c_tmp<metaprogramming::is_const_size<T1>::value, T1>
        {};

        template <typename T>
        class serialize_tmp<true, T>
        {
        public:
            static void   serialize(uint8_t *pos, const T& t) { *reinterpret_cast<T*>(pos) = t; }
            static void deserialize(const uint8_t *pos, T& t) { t = *reinterpret_cast<const T*>(pos); }
        };

        template <typename T>
        class serialize_tmp<false, T> 
        {
            using val_t = typename T::value_type;
            using cref_t = typename T::const_reference;

            using size_calc = size_c_tmp_wrapper<val_t>;

        public:
            static void serialize(uint8_t *pos, const T& t)
            {
                for(cref_t x : t) {
                    serialize_tmp<std::is_fundamental<val_t>::value, val_t>::serialize(pos, x);
                    pos += size_calc::size(x);
                }
            }

            static void deserialize(const uint8_t *pos, T& t)
            {
                for(cref_t x : t) {
                    serialize_tmp<std::is_fundamental<val_t>::value, val_t>::deserialize(pos, x);
                    pos += size_calc::size(x);
                }
            }
        };

        template <typename T>
        class serialize_tmp_wrapper:
            public serialize_tmp<std::is_fundamental<T>::value, T>
        {};

        template <typename T0, typename T1>
        class serialize_tmp_wrapper<pair<T0, T1>>:
            public serialize_tmp<std::is_fundamental<T1>::value, T1>
        {};

        template <typename ...>
        class serializer;

        template <typename T, typename ...Tail>
        class serializer<T, pattern<Tail...>>:
            protected size_c_tmp_wrapper<T>,
            protected serialize_tmp_wrapper<T>
        {};

        template <>
        class serializer<pattern<>, pattern<>>
        {
        protected:
            static void serialize(uint8_t*){}
            static void deserialize(const uint8_t*){}
            static size_t size() { return 0; }
        };

        template <typename ...Args, typename ...FArgs, typename F, F U>
        class serializer<pattern<std::integral_constant<F, U>, Args...>, pattern<FArgs...>>:
            serializer<F, pattern<Args...>>,
            serializer<pattern<Args...>, pattern<FArgs...>>
        {
            using head = serializer<F, pattern<Args...>>;
            using tail = serializer<pattern<Args...>, pattern<FArgs...>>;

        protected:
            static void serialize(uint8_t *dst, const FArgs&... args)
            {
                head::serialize(dst, U);
                tail::serialize(dst + head::size(U), args...);
            }

            static void deserialize(const uint8_t *dst, FArgs&... args)
            {
                //head::deserialize(dst, U); TODO add check
                tail::deserialize(dst + head::size(U), args...);
            }

            static size_t size(const FArgs&... args)
            {
                return tail::size(args...) + head::size(U);
            }
        };

        template <typename ...Args, typename ...FArgs, typename F>
        class serializer<pattern<F, Args...>, pattern<F, FArgs...>>:
            serializer<F, pattern<Args...>>,
            serializer<pattern<Args...>, pattern<FArgs...>>
        {
            using head = serializer<F, pattern<Args...>>;
            using tail = serializer<pattern<Args...>, pattern<FArgs...>>;

        protected:
            static void serialize(uint8_t *dst, const F& f, const FArgs&... args)
            {
                head::serialize(dst, f);
                tail::serialize(dst + head::size(f), args...);
            }

            static void deserialize(const uint8_t *dst, F& f, FArgs&... args)
            {
                head::deserialize(dst, f);
                tail::deserialize(dst + head::size(f), args...);
            }

            static size_t size(const F& f, const FArgs&... args)
            {
                return tail::size(args...) + head::size(f);
            }
        };

        template <typename ...Args, typename ...FArgs, typename T0, typename T1>
        class serializer<pattern<pair<T0, T1>, Args...>, pattern<FArgs...>>:
            protected serializer<pattern<T1, Args...>, pattern<FArgs...>> {};

        template <typename ...Args, typename ...FArgs, typename ...SubSequenceArgs>
        class serializer<pattern<pattern<SubSequenceArgs...>, Args...>, pattern<FArgs...>>:
            protected serializer<pattern<SubSequenceArgs..., Args...>, pattern<FArgs...>> {};
    }

    namespace package_buffer
    {
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
        class buffer<false, pattern<NonConstArgs...>, Args...>:
            serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>> 
        {
            const size_t d_size;
            uint8_t *data;

        public:
            buffer(const NonConstArgs&... args):
                d_size(this->size(args...)),
                data(new uint8_t[d_size])
            {
                this->serialize(data, args...);
            }

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args, typename ...NonConstArgs>
        class buffer<true, pattern<NonConstArgs...>, Args...>:
            public compile_time_size_calc_util<Args...>,
            serialization::serializer<pattern<Args...>, pattern<NonConstArgs...>>
        {
            uint8_t data[size<Args...>::value];

        public:
            buffer(const NonConstArgs&... args)
            {
                this->serialize(data, args...);
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
