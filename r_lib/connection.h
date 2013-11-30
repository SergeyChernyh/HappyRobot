#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "dimension.h"

namespace robot
{

// abstract socket wrapper

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

    int read (      char* data, int size) { return socket.read (data, size); }
    int write(const char* data, int size) { return socket.write(data, size); }
};

// additional type traits

template <typename T>
struct is_constant_size : std::is_fundamental<T> {};

template <typename T, T C>
struct is_constant_size <std::integral_constant<T, C>> : std::true_type {};

template <typename Key, typename Data>
struct is_constant_size<pair<Key, Data>>: is_constant_size<Data> {};

template <typename V, typename U>
struct is_constant_size<phis_value<V, U>>: is_constant_size<V> {};

template <typename Head, typename ...Tail>
struct is_constant_size<std::tuple<Head, Tail...>> :
std::integral_constant
<
    bool,
    is_constant_size<         Head    >::value &&
    is_constant_size<std::tuple<Tail...>>::value
>
{};

template <>
struct is_constant_size<std::tuple<>> : std::true_type {};

// empty dst

struct empty_dst {};

template <typename IStream>
inline IStream& operator >> (IStream& is, empty_dst& t)
{
    return is;
}

// connection definition

class connection
{
    std::shared_ptr<socket_wrapper_base> socket;

public:
    template <typename S>
    connection(const S& s): socket(new socket_wrapper<S>(s)) {}

    template <typename T>
    void read(T& t)
    {
        static_assert
        (
            is_constant_size<std::tuple<T>>::value,
            "size of parameter is not compile time constant"
        );

        read(calc_size(t), t);
    }

    template <typename T>
    void read(size_t size, T& t)
    {
        if(size != 0) {
            binary_buffer buffer = read_buffer(size);
            binary_istream is(buffer);
            deserialize(is, t);
        }
    }

    binary_buffer read_buffer(size_t size)
    {
        binary_buffer buffer(size);

        size_t byte_readed = 0;

        if(size != 0)
            byte_readed = socket->read((char*)(buffer.data), size);

        if(byte_readed != size)
            ; // TODO exc

        return buffer;
    }

    template <typename T>
    void write(const T& t)
    {
        binary_buffer buffer(make_buffer(t));
        socket->write((const char*)(buffer.data), buffer.size);
    }
};

}

#endif
