#ifndef __PACKAGE__
#define __PACKAGE__

#include <numeric>

#include "utility/serialization.h"
#include "utility/select.h"
#include "utility/map.h"

namespace robot { namespace package_creation
{
    template <typename ...Args>
    using no_const_args = utility::select<utility::is_no_const, Args...>;

    template <typename ...Args>
    using size = utility::byte_count<size_t, Args...>;

    namespace package_buffer
    {
        namespace m = utility;

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
        struct const_buffer<m::sequence<Args...>>: public const_buffer<Args...> {};

        template <typename ...Args>
        const uint8_t const_buffer<Args...>::data[] = { Args::value... };

        template <bool is_const_size, typename NonConstArgs, typename ...Args>
        class buffer;

        template <typename... Args, typename ...NonConstArgs>
        class buffer<false, m::sequence<NonConstArgs...>, Args...>
        {
            using inserter = m::run_time_serialization_utility::serializer<m::sequence<Args...>>;

            const size_t d_size;
            uint8_t *data;

        public:
            buffer(const NonConstArgs&... args):
                d_size(inserter::size(args...)),
                data(new uint8_t[d_size])
            {
                inserter::insert(data, args...);
            }

            ~buffer() { delete []data; }

            size_t data_size() const { return d_size; }
            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args, typename ...NonConstArgs>
        class buffer<true, m::sequence<NonConstArgs...>, Args...>: public compile_time_size_calc_util<Args...>
        {
            using inserter = m::run_time_serialization_utility::serializer<m::sequence<Args...>>;
            uint8_t data[size<Args...>::value];

        public:
            buffer(const NonConstArgs&... args)
            {
                inserter::insert(data, args...);
            }

            const uint8_t* get_data() const { return data; }
        };

        template <typename... Args>
        class buffer<true, m::sequence<>, Args...>: public const_buffer<m::serialize<Args...>>
        {};
    }

    template <typename ...Args>
    using package =
    package_buffer::buffer
    <
        utility::is_const_size<Args...>::value,
        no_const_args<Args...>,
        Args...
    >;
}}

#endif //__PACKAGE__
