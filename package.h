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

    template <typename ...Args>
    using pattern = utility::sequence<Args...>;

    namespace package_buffer
    {
        namespace calc_size = utility::run_time_calc_size;
        namespace serialization = utility::run_time_serialization;

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
        class buffer<true, pattern<>, Args...>: public const_buffer<utility::serialize<Args...>>
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
