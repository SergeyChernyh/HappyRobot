#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <map>
#include "common_protocol_parser.h"
#include "common_protocol_parameter_map.h"

namespace robot {

template <typename Interface>
class console_client: common_protocol::read_buffer
{
    using input_map_t =
    std::map // function types
    <
        uint16_t,   // function type
        std::vector // same type functions
        <
            std::vector // function parameter
            <
                parameter_map
            >
        >
    >;

    input_map_t input;

    Interface& io;
    uint32_t msg_counter;

    template <uint16_t MsgGroup, uint16_t MsgType, typename ...Args>
    void send_msg(const Args&... args)
    {
        using namespace robot::common_protocol;

        auto size = package_creation::package<message_body<MsgGroup, MsgType>>(args...).data_size();
        auto p = package_creation::package<message<MsgGroup, MsgType>>(msg_counter, size, args...);
        io.write((const char*)(p.get_data()), p.data_size());
        msg_counter++;
    }

    template <uint16_t MsgGroup, uint16_t MsgType>
    void read_known_package()
    {
        uint32_t msg_num;
        uint32_t data_size;
        if(io.read(header_data, header_size) != header_size)
            return; // TODO exc?
        package_creation::parser<common_protocol::message_header<MsgGroup, MsgType>>::parse(header_data, msg_num, data_size);
        io.read(data_array, data_size);
    }

    void parse_function_config()
    {
        using namespace common_protocol;

        uint16_t f_code;
        uint16_t f_num;
        uint8_t param_count;

        using config_head = pattern<uint16_t, uint16_t, uint8_t>;

        package_creation::parser<config_head>::parse(data_array, f_code, f_num, param_count);

        input[f_code].resize(input[f_code].size() + 1); // new function

        const uint8_t* p_config_ptr = data_array + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t);

        for(uint8_t i = 0; i < param_count; i++) {
            size_t shift = parse_parameter_config(f_code, f_num, p_config_ptr);
            p_config_ptr += shift;
        }
    }

    size_t parse_parameter_config(uint16_t f_code, uint16_t f_num, const uint8_t* p_config_ptr)
    {
        using namespace common_protocol;

        uint8_t p_type;
        uint8_t p_code;

        package_creation::parser<pattern<uint8_t, uint8_t>>::parse(p_config_ptr, p_type, p_code);

        size_t shift = sizeof(uint8_t) + sizeof(uint8_t);

        if(p_type == 0)
            return shift;

        uint8_t read_level;
        uint8_t write_level;
        uint32_t field_count;
        uint8_t field_size;
        uint8_t value_flags;

        package_creation::parser<pattern<uint8_t, uint8_t, uint32_t, uint8_t, uint8_t>>::parse(p_config_ptr + shift, read_level, write_level, field_count, field_size, value_flags);

        shift += sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t);

        const uint8_t* ptr = p_config_ptr + shift;

        if(uint8_t(1 << 1) & value_flags) {
            if(field_size == 3)
                shift += insert_input<float>(f_code, f_num, field_count, ptr);
            else if(field_size == 0xff)
                shift += insert_input<double>(f_code, f_num, field_count, ptr);
            else ; //TODO exc
        }
        else {
            if(uint8_t(1 << 0) & value_flags) {
                switch(field_size) {
                case 0: shift += insert_input<int8_t >(f_code, f_num, field_count, ptr); break;
                case 1: shift += insert_input<int16_t>(f_code, f_num, field_count, ptr); break;
                case 2: shift += insert_input<int32_t>(f_code, f_num, field_count, ptr); break;
                case 3: shift += insert_input<int64_t>(f_code, f_num, field_count, ptr); break;
                default:; //TODO exc
                }
            }
            else {
                switch(field_size) {
                case 0: shift += insert_input<uint8_t >(f_code, f_num, field_count, ptr); break;
                case 1: shift += insert_input<uint16_t>(f_code, f_num, field_count, ptr); break;
                case 2: shift += insert_input<uint32_t>(f_code, f_num, field_count, ptr); break;
                case 3: shift += insert_input<uint64_t>(f_code, f_num, field_count, ptr); break;
                default:; //TODO exc
                }
            }
        }

        return shift;
    }

    template <typename T>
    size_t insert_input(uint16_t f_code, uint16_t f_num, uint32_t field_count, const uint8_t* ptr)
    {
        using namespace common_protocol;

        T min, max, step;

        uint16_t phis_dimension;

        package_creation::parser<pattern<T, T, T, uint16_t>>::parse(ptr, max, min, step, phis_dimension);

        input[f_code][f_num].push_back(parameter_map(field_count, max, min, step, phis_dimension));
        return 3 * sizeof(T) + sizeof(uint16_t);
    }

    void get_function_list()
    {
        send_msg<0x1, 0x2>();
        read_known_package<0x1, 0x4>();

        using function_list_t = common_protocol::message_body<0x1, 0x4>;
        metaprogramming::at_c<0, function_list_t> function_list;

        package_creation::parser<function_list_t>::parse(data_array, function_list);

        for(auto& p : function_list) {
            send_msg<0x1, 0x5>(at_c<0>(p), at_c<1>(p));
            read_known_package<0x1, 0x7>();
            parse_function_config();
        }
    }

    void write(uint16_t f_code, uint16_t f_num, const std::vector<uint8_t>& p_code)
    {
        using namespace common_protocol;

        using parameter_with_code_t = pattern<uint8_t, any>;
        repeat<uint8_t, parameter_with_code_t> p;

        for(auto code : p_code)
            p.push_back(parameter_with_code_t(code, input[f_code][f_num][code].get())); 

        send_msg<0x2, 0x8>(f_code, f_num, p);
    }

    void read(uint16_t f_code, uint16_t f_num, const metaprogramming::at_c<0x2, common_protocol::message_body<0x2, 0x0>>& request)
    {
        using namespace common_protocol;

        send_msg<0x2, 0x0>(f_code, f_num, request);

        read_known_package<0x2, 0x6>();
        get_parameters();
    }

    void get_parameters()
    {
        using namespace common_protocol;

        uint16_t f_code;
        uint16_t f_num;
        uint8_t param_count;

        using values_head_t = pattern<uint16_t, uint16_t, uint8_t>;

        package_creation::parser<values_head_t>::parse(data_array, f_code, f_num, param_count);

        size_t shift = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t);

        for(uint8_t i = 0; i < param_count; i++) {
            uint8_t p_code = data_array[shift];
            shift += sizeof(uint8_t);
            auto current_param = input[f_code][f_num][p_code].get();
            package_creation::parser<pattern<decltype(current_param)>>::parse(data_array + shift, current_param);
            shift += current_param.size_c(); // TODO
            shift += sizeof(uint8_t); //TODO parse labels
        }
    }

public:
    console_client(Interface& i):
        io(i),
        msg_counter(0)
    {
        get_function_list();

        while(true) {
            uint16_t f_code;
            uint16_t f_num;
            uint16_t p_code;
            std::cout << "enter function code: ";
            std::cin >> f_code;
            std::cout << "enter function number: ";
            std::cin >> f_num;
            std::cout << "enter parameter code: ";
            std::cin >> p_code;
            std::cout << "enter parameter value: ";

            std::cin >> input[f_code][f_num][p_code];

            using read_reguest = metaprogramming::at_c<0x2, common_protocol::message_body<0x2, 0x0>>;

            read_reguest req;
            req.push_back(common_protocol::pattern<uint8_t, uint8_t>((uint8_t)p_code, (uint8_t)0));

            write(f_code, f_num, std::vector<uint8_t>(1, p_code));
            read(f_code, f_num, req);

            std::cout << input[f_code][f_num][p_code] << std::endl;
        }
    }
};

}

#endif
