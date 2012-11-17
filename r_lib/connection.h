#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "package.h"

namespace robot { namespace connection {

template <typename ...Args>
using pattern = package_creation::pattern<Args...>;

class socket_wrapper_base
{
public:
    virtual int read (      char* data, int size) = 0;
    virtual int write(const char* data, int size) = 0;
};

template <typename SocketType>
class socket_wrapper : public socket_wrapper_base
{
    SocketType socket;
public:
    socket_wrapper(const SocketType& s): socket(s) {}

    int read (char* data, int size)
    {
        return socket.read (data, size);
    }

    int write(const char* data, int size)
    {
        return socket.write(data, size);
    }
};

class connection
{
    template <size_t C>
    struct const_size_buffer
    {
        uint8_t data[C];
    };

    struct buffer
    {
        uint8_t *data;
        buffer(size_t size): data(new uint8_t[size]) {}
        ~buffer() { delete[] data; }
    };

    std::shared_ptr<socket_wrapper_base> socket;

public:
    template <typename S>
    connection(const S& s): socket(new socket_wrapper<S>(s)) {}

    template <typename ...Args>
    void read(Args&... args)
    {
        static_assert
        (
            metaprogramming::is_const_size<Args...>::value,
            "Args size is not constant. Use sized_read"
        );

        constexpr size_t read_size = package_creation::size<Args...>::value;
        const_size_buffer<read_size> buf;

        socket->read((char*)(buf.data), read_size);

        package_creation::parser<Args...>::parse(buf.data, args...);
    }

    template <typename ...Args>
    void sized_read(size_t read_size, Args&... args)
    {
        buffer buf[read_size];
        socket->read((char*)(buf.data), read_size);
        package_creation::parser<Args...>::parse(buf.data, args...);
    }

    template <typename ...Args>
    void write(const Args&... args)
    {
        package_creation::package<pattern<Args...>> p(args...);
        socket->write((const char*)(p.get_data()), p.data_size());
    }
};

}}

#endif
