#ifndef __COMMON_PROTOCOL_FUNCTION__
#define __COMMON_PROTOCOL_FUNCTION__

#include "protocol_definition_names.h"
#include "parameter.h"
#include "package.h"

namespace robot { namespace common_protocol
{
    using namespace protocol_definition_names;

    struct parameter_rw_base
    {
        virtual void set_val(const uint8_t* src) = 0;
        virtual size_t size() const = 0;
    };

    template <typename T>
    struct fields_count
    {
        constexpr static uint32_t num(const T& t) { return 1; }
    };

    template <typename T>
    struct container_fields_count
    {
        static uint32_t num(const T& t) { return t.size(); }
    };

    template <typename T>
    struct fields_count<std::vector<T>>: container_fields_count<std::vector<T>> {};
    template <typename T, size_t C>
    struct fields_count<std::array<T, C>>: container_fields_count<std::array<T, C>> {};

    template <typename>
    class parameter_rw;

    template
    <
        typename T,
        typename Type,
        uint8_t READ_LEVEL,
        uint8_t WRITE_LEVEL,
        subsystem::details::value_type<T> MIN,
        subsystem::details::value_type<T> MAX,
        subsystem::details::value_type<T> STEP
    >
    class parameter_rw<subsystem::parameter<T, Type, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>>: parameter_rw_base
    {
        using parameter_t = subsystem::parameter<T>;
        parameter_t& parameter;

        T write_val;

        using config_t =
        pattern
        <
            metaprogramming::at_key
            <
                Type,
                metaprogramming::pair<subsystem::NA, std::integral_constant<uint8_t, 0>>,
                metaprogramming::pair<subsystem::RO, std::integral_constant<uint8_t, 1>>,
                metaprogramming::pair<subsystem::WO, std::integral_constant<uint8_t, 2>>,
                metaprogramming::pair<subsystem::RW, std::integral_constant<uint8_t, 3>>
            >,
            uint8_t,
            std::integral_constant<uint8_t, READ_LEVEL>,
            std::integral_constant<uint8_t, WRITE_LEVEL>,
            uint32_t,
            metaprogramming::at_key
            <
                std::integral_constant<size_t, sizeof(subsystem::details::value_type<T>)>,
                metaprogramming::pair<std::integral_constant<size_t, 1>, std::integral_constant<uint8_t, 0>>,
                metaprogramming::pair<std::integral_constant<size_t, 2>, std::integral_constant<uint8_t, 1>>,
                metaprogramming::pair<std::integral_constant<size_t, 4>, std::integral_constant<uint8_t, 2>>,
                metaprogramming::pair<std::integral_constant<size_t, 8>, std::integral_constant<uint8_t, 3>>,
                metaprogramming::pair<std::integral_constant<size_t, 10>, std::integral_constant<uint8_t, 0xff>>
            >
        >;

    public:
        parameter_rw(parameter_t& p): parameter(p) {}

        size_t size() const { return pack<pattern<T>>(write_val).data_size(); }

        void set_val(const uint8_t* src)
        {
            package_creation::parser<pattern<T>>::parse(src, write_val);
        }
    };

    class function_parameter
    {
    };
}}

#endif //__COMMON_PROTOCOL_FUNCTION__
