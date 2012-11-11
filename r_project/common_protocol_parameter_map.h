#ifndef __COMMON_PROTOCOL_PARAMETER_MAP_H__
#define __COMMON_PROTOCOL_PARAMETER_MAP_H__

#include <iostream>
#include <vector>

namespace robot {

class virtual_console_io_node
{
    virtual void input_value (std::istream& is)       = 0;
    virtual void output_value(std::ostream& os) const = 0;
public:

    virtual common_protocol::any get() = 0;

    friend std::istream& operator >>(std::istream& is, virtual_console_io_node& v)
    {
        v.input_value(is);
        return is;
    }

    friend std::ostream& operator <<(std::ostream& os, virtual_console_io_node& v)
    {
        v.output_value(os);
        return os;
    }
};

template <typename T>
class console_io_node : public virtual_console_io_node
{
    std::vector<T> v;

    void input_value (std::istream& is)       { for(auto& p : v) is >> p; }
    void output_value(std::ostream& os) const { for(auto& p : v) os << p; }
public:
    console_io_node(uint32_t size) { v.resize(size); }

    common_protocol::any get() { return v; }
};

template <typename T>
class char_console_io_node : public virtual_console_io_node
{
    std::vector<T> v;

    void input_value (std::istream& is)       { for(auto& p : v) { is >> p; p -= '0'; } }
    void output_value(std::ostream& os) const { for(auto& p : v)   os << p; } // TODO
public:
    char_console_io_node(uint32_t size) { v.resize(size); }

    common_protocol::any get() { return v; }
};

template <> class console_io_node< int8_t> : public char_console_io_node< int8_t> { public: console_io_node(uint32_t size) : char_console_io_node< int8_t>(size) {} };
template <> class console_io_node<uint8_t> : public char_console_io_node<uint8_t> { public: console_io_node(uint32_t size) : char_console_io_node<uint8_t>(size) {} };

}
#endif
