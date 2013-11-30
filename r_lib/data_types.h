#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <cstdint>   // uint8_t ...
#include <cstring>   // size_t
#include <cmath>     // pow
#include <iostream>  // std::ostream

#include <tuple>     // std::tuple
#include <memory>    // std::shared_ptr
#include <vector>    // std::vector
#include <array>     // std::array
#include <algorithm> // std::copy
#include <stdexcept> // std::logic_error, std::out_of_range

namespace robot {

///////////////////////////////////////////////////////////
//
//                  serialization
//
///////////////////////////////////////////////////////////

template <typename IStream>
inline void deserialize(IStream&) {}

template <typename IStream, typename Head, typename ...Tail>
inline void deserialize(IStream& is, Head& head, Tail&... tail)
{
    is >> head;
    deserialize(is, tail...);
}

template <typename OStream>
static void serialize(const OStream&) {}

template <typename OStream, typename Head, typename ...Tail>
inline void serialize(OStream& os, const Head& head, const Tail&... tail)
{
    os << head;
    serialize(os, tail...);
}

///////////////////////////////////////////////////////////
//
//              int8 serialization diff
//
///////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& os, const uint8_t& p)
{
    uint16_t res = p;
    return os << res;
}

inline std::ostream& operator<<(std::ostream& os, const int8_t& p)
{
    int16_t res = p;
    return os << res;
}

inline std::istream& operator>>(std::istream& is, uint8_t& p)
{
    uint16_t res;
    is >> res;
    p = res;
    return is;
}

inline std::istream& operator>>(std::istream& is, int8_t& p)
{
    int16_t res;
    is >> res;
    p = res;
    return is;
}

///////////////////////////////////////////////////////////
//
//          binary streams for serialization
//
///////////////////////////////////////////////////////////

class size_calc_stream
{
    size_t size;
public:
    size_calc_stream() : size(0) {}

    size_t get() const { return size; }

    template <typename T>
    typename
    std::enable_if
    <
        std::is_arithmetic<T>::value,
        size_calc_stream&
    >::type
    operator << (const T& t)
    {
        size += sizeof(T);
        return *this;
    }
};

template <typename T>
inline size_t calc_size(const T& t)
{
    size_calc_stream s;
    s << t;
    return s.get();
}

template <typename Head, typename ...Tail>
inline size_t calc_size(const Head& head, const Tail&... tail)
{
    return calc_size(head) + calc_size(tail...);
}

struct binary_buffer
{
    const size_t size;
    char *data;

    binary_buffer(size_t s) :
        size(s),
        data(new char[s])
    {}

    binary_buffer(const binary_buffer& b):
        size(b.size),
        data(new char[size])
    {
        std::copy(b.data, b.data + size, data);
    }

    ~binary_buffer() { delete [] data; }
};

class binary_stream_base
{
    binary_buffer& buffer;
protected:
    char *ptr;

    template <typename T>
    void check_overflow()
    {
        if(ptr - buffer.data + sizeof(T) > buffer.size)
            throw std::out_of_range("error: bin stream buffer out of range");
    }

    binary_stream_base(binary_buffer& b) :
        buffer(b),
        ptr(buffer.data)
    {}
public:
    const binary_buffer& get() const { return buffer; }
};

class binary_ostream : public binary_stream_base
{
public:
    binary_ostream(binary_buffer& b) : binary_stream_base(b) {}

    template <typename T>
    typename
    std::enable_if
    <
        std::is_arithmetic<T>::value,
        binary_ostream&
    >::type
    operator << (const T& t)
    {
        check_overflow<T>();
        *(T*)(ptr) = t; // TODO big endian
        ptr += sizeof(T);
        return *this;
    }
};

class binary_istream : public binary_stream_base
{
public:
    binary_istream(binary_buffer& b) : binary_stream_base(b) {}

    template <typename T>
    typename
    std::enable_if
    <
        std::is_arithmetic<T>::value &&
        !std::is_const<T>::value,
        binary_istream&
    >::type
    operator >> (T& t)
    {
        check_overflow<T>();
        t = *(T*)(ptr); // TODO big endian
        ptr += sizeof(T);
        return *this;
    }
};

template <typename T>
inline binary_buffer make_buffer(const T& t)
{
    binary_buffer buf(calc_size(t));
    binary_ostream os(buf);
    os << t;
    return buf;
}

///////////////////////////////////////////////////////////
//
//            serialization: constant check
//
///////////////////////////////////////////////////////////

struct constant_mismatch_error: public std::logic_error
{
    constant_mismatch_error():
        std::logic_error("error: const field value mismatch")
    {}
};

template <typename IStream, typename T>
inline
typename
std::enable_if
<
    std::is_arithmetic<T>::value &&
    std::is_const<T>::value,
    IStream&
>::type
operator >> (IStream& is, T& t)
{
    typename std::remove_const<T>::type tmp;
    is >> tmp;
    if(tmp != t)
        throw constant_mismatch_error();
    return is;
}

///////////////////////////////////////////////////////////
//
//          serialization: std::integral_constant
//
///////////////////////////////////////////////////////////


template <typename OStream, typename T, T C>
inline OStream& operator << (OStream& os, const std::integral_constant<T, C>& t)
{
    return os << t.value;
}

template <typename IStream, typename T, T C>
inline IStream& operator >> (IStream& is, std::integral_constant<T, C>& t)
{
    return is >> t.value;
}

///////////////////////////////////////////////////////////
//
//              serialization: containers
//
///////////////////////////////////////////////////////////

// array

template <typename OStream, typename T, size_t C>
inline OStream& operator << (OStream& os, const std::array<T, C>& t)
{
    for(const auto& p : t)
        os << p;
    return os;
}

template <typename IStream, typename T, size_t C>
inline IStream& operator >> (IStream& is, std::array<T, C>& t)
{
    for(auto& p : t)
        is >> p;
    return is;
}

// vector serialization

template <typename OStream, typename T>
inline OStream& operator << (OStream& os, const std::vector<T>& t)
{
    for(const auto& p : t)
        os << p;
    return os;
}

template <typename IStream, typename T>
inline IStream& operator >> (IStream& is, std::vector<T>& t)
{
    for(auto& p : t)
        is >> p;
    return is;
}

// repeat

template <typename SizeType, typename T>
class repeat : public std::vector<T> {};

// repeat serialization

template <typename OStream, typename SizeType, typename T>
inline OStream& operator << (OStream& os, const repeat<SizeType, T>& t)
{
    SizeType size = t.size();
    os << size;
    const std::vector<T>& vec = t;
    return os << vec;
}

template <typename IStream, typename SizeType, typename T>
inline IStream& operator >> (IStream& is, repeat<SizeType, T>& t)
{
    SizeType size = 0;
    is >> size;
    t.resize(size);
    std::vector<T>& vec = t;
    return is >> vec;
}

///////////////////////////////////////////////////////////
//
//     std::tuple : serialization & asotiative access
//
///////////////////////////////////////////////////////////

namespace details
{

// tuple serialization

template <size_t INDEX, typename OStream, typename ...T>
inline
typename std::enable_if<INDEX == sizeof...(T), void>::type
tuple_serialize(OStream& os, const std::tuple<T...>& t) {}

template <size_t INDEX, typename IStream, typename ...T>
inline
typename std::enable_if<INDEX == sizeof...(T), void>::type
tuple_deserialize(IStream& is, std::tuple<T...>& t) {}

template <size_t INDEX, typename OStream, typename ...T>
inline
typename std::enable_if<INDEX < sizeof...(T), void>::type
tuple_serialize(OStream& os, const std::tuple<T...>& t)
{
    os << std::get<INDEX>(t);
    tuple_serialize<INDEX + 1>(os, t);
}

template <size_t INDEX, typename IStream, typename ...T>
inline
typename std::enable_if<INDEX < sizeof...(T), void>::type
tuple_deserialize(IStream& is, std::tuple<T...>& t)
{
    is >> std::get<INDEX>(t);
    tuple_deserialize<INDEX + 1>(is, t);
}

}

template <typename OStream, typename ...T>
inline OStream& operator << (OStream& os, const std::tuple<T...>& t)
{
    details::tuple_serialize<0>(os, t);
    return os;
}

template <typename IStream, typename ...T>
inline IStream& operator >> (IStream& is, std::tuple<T...>& t)
{
    details::tuple_deserialize<0>(is, t);
    return is;
}

// pair for associative element access 

template <typename Key, typename Val>
struct pair
{
    using value_t = Val;
    Val value;
};

template <typename OStream, typename Key, typename Val>
inline OStream& operator << (OStream& os, const pair<Key, Val>& t)
{
    return os << t.value;
}

template <typename IStream, typename Key, typename Val>
inline IStream& operator >> (IStream& is, pair<Key, Val>& t)
{
    return is >> t.value;
}

// tuple associative access

namespace details
{

template <size_t C, typename Key, typename ...T>
struct tuple_key_compare_index;

template <size_t C, typename Key>
struct tuple_key_compare_index<C, Key> {};

template <size_t C, typename Key, typename Head, typename ...T>
struct tuple_key_compare_index<C, Key, Head, T...> :
    tuple_key_compare_index<C + 1, Key, T...>
{
    static_assert(sizeof...(T) != 0, "out of range");
};

template <size_t C, typename Key, typename V, typename ...T>
struct tuple_key_compare_index<C, Key, pair<Key, V>, T...> :
    std::integral_constant<size_t, C>
{};

template <typename Key, typename ...T>
struct key_index : tuple_key_compare_index<0, Key, T...> {};

}

template <typename Key, typename T>
class tuple_element;

template <typename Key, typename ...T>
struct tuple_element<Key, std::tuple<T...>>
{
    using type =
    typename std::tuple_element
    <
        details::key_index<Key, T...>::value,
        std::tuple<T...>
    >::type::value_t;
};

template <typename Key, typename T>
using at_key = typename tuple_element<Key, T>::type;

template <typename Key, typename ...T>
inline at_key<Key, std::tuple<T...>>& get(std::tuple<T...>& t)
{
    constexpr size_t i = details::key_index<Key, T...>::value;
    return std::get<i>(t).value;
}

template <typename Key, typename ...T>
inline const at_key<Key, std::tuple<T...>>& get(const std::tuple<T...>& t)
{
    constexpr size_t i = details::key_index<Key, T...>::value;
    return std::get<i>(t).value;
}

///////////////////////////////////////////////////////////
//
//                          any
//
///////////////////////////////////////////////////////////

class storage_base
{
public:
    virtual void write(std::ostream&) const = 0;
    virtual void read(std::istream&)        = 0;

    virtual void write(binary_ostream&) const = 0;
    virtual void read(binary_istream&)        = 0;

    virtual void write(size_calc_stream&) const = 0;

    virtual ~storage_base() {}
};

template <typename T>
class storage : public storage_base
{
    T val;
public:
    storage(const T& t) : val(t) {}

    virtual void write(std::ostream& os) const { os << val; }
    virtual void read (std::istream& is)       { is >> val; }

    virtual void write(binary_ostream& os) const { os << val; }
    virtual void read (binary_istream& is)       { is >> val; }

    virtual void write(size_calc_stream& calc) const { calc << val; }
};

template <typename T>
class storage_ref : public storage_base
{
    T& val;
public:
    storage_ref(T& t) : val(t) {}

    virtual void write(std::ostream& os) const { os << val; }
    virtual void read (std::istream& is)       { is >> val; }

    virtual void write(binary_ostream& os) const { os << val; }
    virtual void read (binary_istream& is)       { is >> val; }

    virtual void write(size_calc_stream& calc) const { calc << val; }
};

class any
{
    std::shared_ptr<storage_base> value;
public:
    template <typename T> any(storage    <T>* t) : value(t) {}
    template <typename T> any(storage_ref<T>* t) : value(t) {}

    template <typename OStream>
    void write(OStream& os) const { value->write(os); }

    template <typename IStream>
    void read (IStream& is) { value->read (is); }
};

template <typename T>
inline any make_storage(const T& t)
{
    return any(new storage<T>(t));
}

template <typename T>
inline any make_storage_ref(T& t)
{
    return any(new storage_ref<T>(t));
}

// any serialization

template <typename OStream>
inline OStream& operator << (OStream& os, const any& t)
{
    t.write(os);
    return os;
}

template <typename IStream>
inline IStream& operator >> (IStream& is, any& t)
{
    t.read(is);
    return is;
}

}

#endif
