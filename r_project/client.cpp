#include <iostream>
#include <vector>
#include <map>
#include "tcp.h"
#include "common_protocol_parser.h"

using namespace robot;

class virtual_console_io_node
{
public:
    virtual common_protocol::any read_value() = 0;
};

template <typename T>
class console_io_node : public virtual_console_io_node
{
    std::vector<T> v;
public:
    console_io_node(uint32_t size) { v.resize(size); }

    common_protocol::any read_value()
    {
        for(auto& p : v)
            std::cin >> p;
        return v;
    }
};

template <typename T>
class char_console_io_node : public virtual_console_io_node
{
    std::vector<T> v;
public:
    char_console_io_node(uint32_t size) { v.resize(size); }

    common_protocol::any read_value()
    {
        for(auto& p : v) {
            std::cin >> p;
            p -= '0';
        }
        return v;
    }
};

template <> class console_io_node< int8_t> : public char_console_io_node< int8_t> { public: console_io_node(uint32_t size) : char_console_io_node< int8_t>(size) {} };
template <> class console_io_node<uint8_t> : public char_console_io_node<uint8_t> { public: console_io_node(uint32_t size) : char_console_io_node<uint8_t>(size) {} };

template <typename Interface>
class console_client: common_protocol::read_buffer
{
    using input_map_t = std::map<uint16_t, std::vector<std::vector<std::shared_ptr<virtual_console_io_node>>>>;
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
            return;
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

        if(uint8_t(1 << 1) & value_flags) {
            if(field_size == 3)
                shift += insert_input<float>(f_code, f_num, field_count);
            else if(field_size == 0xff)
                shift += insert_input<double>(f_code, f_num, field_count);
            else ; //TODO exc
        }
        else {
            if(uint8_t(1 << 0) & value_flags) {
                switch(field_size) {
                case 0: shift += insert_input<int8_t >(f_code, f_num, field_count); break;
                case 1: shift += insert_input<int16_t>(f_code, f_num, field_count); break;
                case 2: shift += insert_input<int32_t>(f_code, f_num, field_count); break;
                case 3: shift += insert_input<int64_t>(f_code, f_num, field_count); break;
                default:; //TODO exc
                }
            }
            else {
                switch(field_size) {
                case 0: shift += insert_input<uint8_t >(f_code, f_num, field_count); break;
                case 1: shift += insert_input<uint16_t>(f_code, f_num, field_count); break;
                case 2: shift += insert_input<uint32_t>(f_code, f_num, field_count); break;
                case 3: shift += insert_input<uint64_t>(f_code, f_num, field_count); break;
                default:; //TODO exc
                }
            }
        }

        return shift;
    }

    template <typename T>
    size_t insert_input(uint16_t f_code, uint16_t f_num, uint32_t field_count)
    {
        input[f_code][f_num].push_back(std::shared_ptr<virtual_console_io_node>(new console_io_node<T>(field_count)));
        return 3 * sizeof(T) + sizeof(uint16_t); // MIN, MAX, STEP, PHIS VALUE CODE ---> TODO
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

public:
    console_client(Interface& i):
        io(i),
        msg_counter(0)
    {
        using namespace common_protocol;

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

            using parameter_with_code_t = pattern<uint8_t, any>;
            repeat<uint8_t, parameter_with_code_t> p;
            p.push_back(parameter_with_code_t((uint8_t)p_code, input[f_code][f_num][p_code]->read_value())); 

            send_msg<0x2, 0x8>(f_code, f_num, p);
        }
    }
};

int main(int argc, const char* argv[])
{
    using namespace subsystem;

    tcp_test::TCPInterface tcp(5200);

    tcp.client_connect();

    console_client<tcp_test::TCPInterface> client(tcp);

    return 0;
}
