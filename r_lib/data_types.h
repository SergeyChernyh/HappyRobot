#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <cstdint>   // uint8_t ...
#include <cstring>   // size_t
#include <cmath>     // pow
#include <iostream>  // std::ostream

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
//                      sequence
//
///////////////////////////////////////////////////////////

template <typename ...>
struct sequence;

// pair for associative element access 

template <typename Key, typename Val>
struct pair;

// sequence element

template <typename T>
struct sequence_element
{
    using type = T;
    T value;

    template <typename A>
    sequence_element(const A& v): value(v) {}
    sequence_element() {}
};

template <typename T, T C>
struct sequence_element<std::integral_constant<T, C>> :
                        std::integral_constant<T, C>
{
    using type = const T;
    sequence_element(const std::integral_constant<T, C>& v) {}
    sequence_element() {}
};

template <typename Key, typename Val>
struct sequence_element<pair<Key, Val>> :
       sequence_element<          Val>
{
    template <typename A>
    sequence_element(const A& v): sequence_element<Val>(v) {}
    sequence_element() {}
};

// sequence element "tail" - signature for element access

template <typename ...T>
struct sequence_tail;

template <typename Head, typename ...Signature>
struct sequence_tail<Head, Signature...> : sequence_element<Head>
{
    template <typename A>
    sequence_tail(const A& v): sequence_element<Head>(v) {}

    sequence_tail() {}
};

template <>
struct sequence_tail<> {};

// additional type traits for compile time constants skipping
// in sequence constructor parameter list

template <typename T>
struct is_constexpr : std::is_const<typename sequence_element<T>::type> {};

template <>
struct is_constexpr <sequence<>> : std::true_type {};

template <typename Head, typename ...Tail>
struct is_constexpr <sequence<Head, Tail...>> : 
std::integral_constant
<
    bool,
    is_constexpr<Head>::value &&
    is_constexpr<sequence<Tail...>>::value
>
{};

// sequence construct distinctionst

template <bool IsConstexpr, typename Head, typename ...Tail>
struct sequence_construct_option;

// for constexpr args

template <typename Head, typename ...Tail>
struct sequence_construct_option<true, Head, Tail...>:
sequence_tail<Head, Tail...>,
sequence<Tail...>
{
    sequence_construct_option() {}

    template <typename ...Args>
    sequence_construct_option(const Args&... args):
        sequence<Tail...>(args...)
    {}

    template <typename ...Args>
    sequence_construct_option(const Head& p, const Args&... args):
        sequence<Tail...>(args...)
    {}
};

// for runtime args

template <typename Head, typename ...Tail>
struct sequence_construct_option<false, Head, Tail...>:
sequence_tail<Head, Tail...>,
sequence<Tail...>
{
    sequence_construct_option() {}

    template <typename A, typename ...Args>
    sequence_construct_option(const A& a, const Args&... args):
        sequence_tail<Head, Tail...>(a),
        sequence<Tail...>(args...)
    {}
};

// sequence definition

template <>
struct sequence <> {};

template <typename Head, typename ...Tail>
struct sequence<Head, Tail...> :
sequence_construct_option<is_constexpr<Head>::value, Head, Tail...>
{
    static const bool is_const = is_constexpr<Head>::value;

    template <typename ...Args>
    sequence(const Args&... args):
        sequence_construct_option<is_const, Head, Tail...>(args...)
    {}
};

// sequence element type by index

template <size_t C, typename T>
struct type_at_c_;

template <typename Head, typename ...Tail>
struct type_at_c_<0, sequence<Head, Tail...>>
{
    using type = Head;
};

template <typename Key, typename Data, typename ...Tail>
struct type_at_c_<0, sequence<pair<Key, Data>, Tail...>>
{
    using type = Data;
};

template <size_t C, typename Head, typename ...Tail>
struct type_at_c_<C    , sequence<Head, Tail...>> :
       type_at_c_<C - 1, sequence<      Tail...>> {};

template <size_t C, typename T>
using type_at_c = typename type_at_c_<C, T>::type;

template <size_t C, typename T>
using value_type_at_c = typename sequence_element<type_at_c<C, T>>::type;

// sequence element type by key

template <typename Key, typename>
struct type_at_key_;

template <typename Key, typename Data, typename ...Tail>
struct type_at_key_<Key, sequence<pair<Key, Data>, Tail...>>
{
    using type = Data;
};

template <typename Key, typename Head, typename ...Tail>
struct type_at_key_<Key, sequence<Head, Tail...>> :
       type_at_key_<Key, sequence<      Tail...>> {};

template <typename Key, typename T>
using type_at_key = typename type_at_key_<Key, T>::type;

template <typename Key, typename T>
using value_type_at_key =
typename sequence_element<type_at_key<Key, T>>::type;

// sequence tail by index

template <size_t C, typename>
struct tail_at_c_;

template <typename Head, typename ...Tail>
struct tail_at_c_<0, sequence<Head, Tail...>>
{
    using type = sequence_tail<Head, Tail...>;
};

template <size_t C, typename Head, typename ...Tail>
struct tail_at_c_<C    , sequence<Head, Tail...>> :
       tail_at_c_<C - 1, sequence<      Tail...>> {};

template <size_t C, typename T>
using tail_at_c = typename tail_at_c_<C, T>::type;

// sequence tail by key

template <typename Key, typename>
struct tail_at_key_;

template <typename Key, typename Data, typename ...Tail>
struct tail_at_key_<Key, sequence<pair<Key, Data>, Tail...>>
{
    using type = sequence_tail<pair<Key, Data>, Tail...>;
};

template <typename Key, typename Head, typename ...Tail>
struct tail_at_key_<Key, sequence<Head, Tail...>> :
       tail_at_key_<Key, sequence<      Tail...>> {};

template <typename Key, typename T>
using tail_at_key = typename tail_at_key_<Key, T>::type;

// sequence get element by index

template <size_t C, typename ...T>
value_type_at_c<C, sequence<T...>>& get(sequence<T...>& t)
{
    tail_at_c<C, sequence<T...>>& p = t;
    return p.value;
}

template <size_t C, typename ...T>
const value_type_at_c<C, sequence<T...>>& get(const sequence<T...>& t)
{
    const tail_at_c<C, sequence<T...>>& p = t;
    return p.value;
}

// sequence get element by key

template <typename Key, typename T>
value_type_at_key<Key, T>& get(T& t)
{
    tail_at_key<Key, T>& p = t;
    return p.value;
}

template <typename Key, typename T>
const value_type_at_key<Key, T>& get(const T& t)
{
    const tail_at_key<Key, T>& p = t;
    return p.value;
}

// sequence serialization

template <typename OStream>
OStream& operator << (OStream& os, const sequence<>&)
{
    return os;
}

template <typename OStream, typename Head, typename ...Tail>
OStream& operator << (OStream& os, const sequence<Head, Tail...>& t)
{
    const sequence<Tail...>& tail = t;
    os << get<0>(t) << tail;
    return os;
}

template <typename IStream>
IStream& operator >> (IStream& is, const sequence<>&)
{
    return is;
}

template <typename IStream, typename Head, typename ...Tail>
IStream& operator >> (IStream& is, sequence<Head, Tail...>& t)
{
    sequence<Tail...>& tail = t;
    is >> get<0>(t) >> tail;
    return is;
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
