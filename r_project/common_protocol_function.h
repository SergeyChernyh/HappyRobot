#ifndef __COMMON_PROTOCOL_FUNCTION__
#define __COMMON_PROTOCOL_FUNCTION__

#include <vector>
#include <map>

#include "common_protocol.h"
#include "parameter.h"
#include "package.h"

namespace robot { namespace common_protocol
{
    struct parameter_rw_base
    {
        virtual void set_val(const uint8_t* src) = 0;
        virtual size_t size() const = 0;
        virtual any get_config() = 0;
        virtual any get_val() = 0;
        virtual void apply_change() = 0;
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
    class parameter_rw<subsystem::parameter<T, Type, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>> : public parameter_rw_base
    {
        using value_t = subsystem::details::value_type<T>;

        using parameter_t = subsystem::parameter<T>;
        parameter_t& parameter;

        T write_val;
        T read_val;

        using config_t =
        pattern
        <
            metaprogramming::at_key
            <
                Type,
                pair<subsystem::NA, constant<uint8_t, 0>>,
                pair<subsystem::RO, constant<uint8_t, 1>>,
                pair<subsystem::WO, constant<uint8_t, 2>>,
                pair<subsystem::RW, constant<uint8_t, 3>>
            >,
            uint8_t,
            constant<uint8_t, READ_LEVEL>,
            constant<uint8_t, WRITE_LEVEL>,
            uint32_t,
            metaprogramming::at_key
            <
                constant<size_t, sizeof(value_t)>,
                pair<constant<size_t, 1>, constant<uint8_t, 0>>,
                pair<constant<size_t, 2>, constant<uint8_t, 1>>,
                pair<constant<size_t, 4>, constant<uint8_t, 2>>,
                pair<constant<size_t, 8>, constant<uint8_t, 3>>,
                pair<constant<size_t, 10>, constant<uint8_t, 0xff>>
            >,
            constant
            <
                uint8_t,
                std::is_floating_point<value_t>::value ? (1 << 1) : 0 |
                std::is_integral      <value_t>::value ? (1 << 0) : 0 
            >,
            constant<value_t, MAX>,
            constant<value_t, MIN>,
            constant<value_t, STEP>,
            constant<uint16_t, 0> //TODO
        >;

        config_t config;

    public:
        parameter_rw(parameter_t& p, uint8_t param_code):
            parameter(p)
        {
            at_c<1>(config) = param_code;
            at_c<4>(config) = fields_count<subsystem::details::data_type<T>>::num(parameter.get());
        }

        any get_config() { return config; }

        size_t size() const { return pack<pattern<T>>(write_val).data_size(); }

        void set_val(const uint8_t* src)
        {
            package_creation::parser<pattern<T>>::parse(src, write_val);
            // TODO check write_val (exc)
        }

        any get_val()
        {
            read_val = parameter.get();
            return read_val;
        }

        void apply_change()
        {
            parameter.set(write_val);
        }
    };

    template
    <
        typename T,
        uint8_t READ_LEVEL,
        uint8_t WRITE_LEVEL,
        subsystem::details::value_type<T> MIN,
        subsystem::details::value_type<T> MAX,
        subsystem::details::value_type<T> STEP
    >
    class parameter_rw<subsystem::parameter<T, subsystem::NA, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>>: parameter_rw_base
    {
        using value_t = subsystem::details::value_type<T>;

        using parameter_t = subsystem::parameter<T, subsystem::NA, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>;
        parameter_t& parameter;

        T write_val;

        using config_t = pattern<constant<uint8_t, 0>, uint8_t>;

        config_t config;

    public:
        parameter_rw(parameter_t& p, uint8_t param_code):
            parameter(p)
        {
            at_c<1>(config) = param_code;
        }

        any get_config() { return config; }

        size_t size() const
        {
            //TODO exc
            return 0;
        }

        void set_val(const uint8_t* src)
        {
            //TODO exc
        }

        any get_val()
        {
            //TODO exc
            return config;
        }

        void apply_change()
        {
            //TODO exc
        }
    };

    class function
    {
        std::vector<std::shared_ptr<parameter_rw_base>> parameters;

        struct inserter
        {
            uint8_t num;
            function& func;

            inserter(function& f): num(0), func(f) {}

            template <typename T>
            void operator() (T& t) { func.add_parameter(t, num); num++; }
        };

    public:
        function() {}

        template <typename ...Args>
        function(subsystem::subsystem<Args...>& s)
        {
            inserter ins(*this);
            algorithm::for_each(ins, s);
        }

        void change_parameters(const uint8_t* src)
        {
            uint8_t p_count = src[0];
            ++src;
            for(uint8_t i = 0; i < p_count; i++) {
                uint8_t p_num = src[0];
                auto& p = parameters[p_num];
                p->set_val(++src);
                src += p->size();
            }

            for(auto& p : parameters)
                p->apply_change();
            
        }

        repeat<uint8_t, any> get_function_config()
        {
            repeat<uint8_t, any> result;
            for(auto& p: parameters)
                result.push_back(p->get_config());
            return result;
        }

        repeat<uint8_t, pattern<any, uint8_t, std::vector<any>>> get_parameter_values(uint8_t* src)
        {
            uint8_t p_count = src[0];
            ++src;
            repeat<uint8_t, pattern<any, uint8_t, std::vector<any>>> result;
            for(uint8_t i = 0; i < p_count; ++i) {
                pattern<uint8_t, std::vector<any>> current;
                result.push_back(pattern<any, uint8_t, std::vector<any>>(parameters[i]->get_val(), uint8_t(0)));
            }
            return result;
        }

        
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
        void add_parameter(subsystem::parameter<T, Type, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>& p, uint8_t param_code)
        {
            parameters.push_back(std::shared_ptr<parameter_rw_base>(new parameter_rw<subsystem::parameter<T, Type, READ_LEVEL, WRITE_LEVEL, MIN, MAX, STEP>>(p, param_code)));
        }
    };

    class function_map
    {
        using vec_t = std::vector<function>;
        using map_t = std::map<uint16_t, vec_t>;
        map_t functions;

        using function_list_t = metaprogramming::at_c<0, message_body<0x1, 0x4>>;

    public:
        void add_function(const function& f, uint16_t f_code) { functions[f_code].push_back(f); }

        vec_t& operator[](uint16_t f_code) { return functions[f_code]; }

        function_list_t get_function_list()
        {
            function_list_t result;

            for(auto& p : functions)
                for(uint16_t i = 0; i < p.second.size(); i++)
                    result.push_back(function_list_t::value_type(uint16_t(p.first), i));

            return result;
        }
    };
}}

#endif //__COMMON_PROTOCOL_FUNCTION__
