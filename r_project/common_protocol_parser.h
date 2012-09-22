#ifndef __COMMON_PROTOCOL_PARSER__
#define __COMMON_PROTOCOL_PARSER__

#include "common_protocol_function.h"

namespace robot { namespace common_protocol
{

class read_buffer
{
protected:
    static constexpr size_t header_size = package_creation::size<message_header_for_parse>::value;
    static constexpr size_t max_body_size = 0x100000;

    uint8_t header_data[header_size];
    uint8_t *data_array;

    read_buffer(): data_array(new uint8_t[max_body_size]) {}
    ~read_buffer() { delete[] data_array; }
};

template <typename Interface>
class server: read_buffer
{
    function_map& functions;
    Interface& io;

    uint32_t msg_counter;

    template <uint16_t MsgGroup, uint16_t MsgType, typename ...Args>
    void send_msg(const Args& ... args)
    {
        auto size = package_creation::package<message_body<MsgGroup, MsgType>>(args...).data_size();
        auto p = package_creation::package<message<MsgGroup, MsgType>>(msg_counter, size, args...);
        io.write((const char*)(p.get_data()), p.data_size());
        msg_counter++;
    }

    void send_function_list()
    {
        send_msg<0x1, 0x4>(functions.get_function_list());
    }

    void send_config()
    {
        uint16_t f_code;
        uint16_t f_num;

        package_creation::parser<message_body<0x1, 0x5>>::parse(data_array, f_code, f_num);
        send_msg<0x1, 0x7>(f_code, f_num, functions[f_code][f_num].get_function_config());
    }

    void send_param_value()
    {
        using value_request_list_t = metaprogramming::at_c<0x2, message_body<0x2, 0x0>>;

        uint16_t f_code;
        uint16_t f_num;

        value_request_list_t value_request_list;

        package_creation::parser<message_body<0x2, 0x0>>::parse(data_array, f_code, f_num, value_request_list);
        send_msg<0x2, 0x6>(f_code, f_num, functions[f_code][f_num].get_parameter_values(value_request_list));
    }

    void change_param_value()
    {
        uint16_t f_code;
        uint16_t f_num;

        package_creation::parser<pattern<uint16_t, uint16_t>>::parse(data_array, f_code, f_num);
        functions[f_code][f_num].change_parameters(data_array + sizeof(uint16_t) + sizeof(uint16_t));
    }

public:
    server(function_map& f, Interface& i):
        functions(f),
        io(i),
        msg_counter(0)
    {}

    void parse()
    {        
        uint16_t msg_group;
        uint16_t msg_type;
        uint32_t msg_num;
        uint32_t data_size;

        if(io.read(header_data, header_size) != header_size) {
            delay(1); // <----- TODO
            return;
        }

        package_creation::parser<message_header_for_parse>::parse(header_data, msg_group, msg_type, msg_num, data_size);

        io.read(data_array, data_size);

        switch(msg_group) {
            case 0x0:
                switch(msg_type) {
                case 0x0:
                case 0x1:
                case 0x2:
                case 0x6:
                case 0x7:
                case 0x8:
                default:; // TODO send error msg
                }
            case 0x1:
                switch(msg_type) {
                case 0x0:
                case 0x1:
                case 0x2: send_function_list(); return;
                case 0x3:
                case 0x4:
                case 0x5: send_config(); return;
                case 0x6:
                case 0x7:
                default:; // TODO send error msg
                }
            case 0x2:
                switch(msg_type) {
                case 0x0: send_param_value(); return;
                case 0x1:
                case 0x2:
                case 0x3:
                case 0x4:
                case 0x5:
                case 0x6:
                case 0x7:
                case 0x8: change_param_value(); return;
                case 0x9:
                case 0x10:
                default:; // TODO send error msg
                }
            default:; // TODO send error msg
        }
    }
};

}}

#endif //__COMMON_PROTOCOL_PARSER__
