#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__

#include <cstdint>   // uint8_t ...
#include <cstring>   // size_t
#include <cmath>     // pow

#include <memory>    // std::shared_ptr
#include <vector>    // std::vector
#include <algorithm> // std::copy
#include <stdexcept> // std::logic_error, std::out_of_range

#include <iostream>

namespace robot {

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
        if(*ptr + sizeof(T) > buffer.size)
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
        !std::is_const<T>::value, // hint for clang
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
    std::is_arithmetic<T>::value,
    IStream&
>::type
operator >> (IStream& is, const T& t)
{
    T tmp;
    is >> tmp;
    if(tmp != t)
        throw constant_mismatch_error();
    return is;
}

///////////////////////////////////////////////////////////
//
//              repeat - specific vector
//
///////////////////////////////////////////////////////////

template <typename SizeType, typename T>
class repeat : public std::vector<T> {};

// repeat serialization

template <typename OStream, typename SizeType, typename T>
inline OStream& operator << (OStream& os, const repeat<SizeType, T>& t)
{
    SizeType size = t.size();
    os << size;
    for(const auto& p : t)
        os << p;
    return os;
}

template <typename IStream, typename SizeType, typename T>
inline IStream& operator >> (IStream& is, repeat<SizeType, T>& t)
{
    SizeType size = 0;
    is >> size;
    t.resize(size);
    for(auto& p : t)
        is >> p;
    return is;
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
    T value;

    template <typename A>
    sequence_element(const A& v): value(v) {}
    sequence_element() {}
};

template <typename T, T C>
struct sequence_element<std::integral_constant<T, C>> :
                        std::integral_constant<T, C>
{
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
struct is_constexpr :
std::is_const<decltype(sequence_element<T>::value)> {};

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
    using type = decltype(sequence_element<Head>::value);
};

template <size_t C, typename Head, typename ...Tail>
struct type_at_c_<C    , sequence<Head, Tail...>> :
       type_at_c_<C - 1, sequence<      Tail...>> {};

template <size_t C, typename T>
using type_at_c = typename type_at_c_<C, T>::type;

// sequence element type by key

template <typename Key, typename>
struct type_at_key_;

template <typename Key, typename Data, typename ...Tail>
struct type_at_key_<Key, sequence<pair<Key, Data>, Tail...>>
{
    using type = decltype(sequence_element<Data>::value);
};

template <typename Key, typename Head, typename ...Tail>
struct type_at_key_<Key, sequence<Head, Tail...>> :
       type_at_key_<Key, sequence<      Tail...>> {};

template <typename Key, typename T>
using type_at_key = typename type_at_key_<Key, T>::type;

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
type_at_c<C, sequence<T...>>& get(sequence<T...>& t)
{
    tail_at_c<C, sequence<T...>>& p = t;
    return p.value;
}

template <size_t C, typename ...T>
const type_at_c<C, sequence<T...>>& get(const sequence<T...>& t)
{
    const tail_at_c<C, sequence<T...>>& p = t;
    return p.value;
}

// sequence get element by key

template <typename Key, typename T>
type_at_key<Key, T>& get(T& t)
{
    tail_at_key<Key, T>& p = t;
    return p.value;
}

template <typename Key, typename T>
const type_at_key<Key, T>& get(const T& t)
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
    virtual void write(std::ostream&) = 0;
    virtual void read(std::istream&) = 0;

    virtual void write(binary_ostream&) = 0;
    virtual void read(binary_istream&) = 0;

    virtual void write(size_calc_stream&) = 0;

    virtual ~storage_base() {}
};

template <typename T>
class storage : public storage_base
{
    T val;
public:
    storage(const T& t) : val(t) {}

    virtual void write(std::ostream& os) { os << val; }
    virtual void read (std::istream& is) { is >> val; }

    virtual void write(binary_ostream& os) { os << val; }
    virtual void read (binary_istream& is) { is >> val; }

    virtual void write(size_calc_stream& calc) { calc << val; }
};

template <typename T>
class storage_ref : public storage_base
{
    T& val;
public:
    storage_ref(T& t) : val(t) {}

    virtual void write(std::ostream& os) { os << val; }
    virtual void read (std::istream& is) { is >> val; }

    virtual void write(binary_ostream& os) { os << val; }
    virtual void read (binary_istream& is) { is >> val; }

    virtual void write(size_calc_stream& calc) { calc << val; }
};

class any
{
    std::shared_ptr<storage_base> value;
public:
    template <typename T> any(storage    <T>* t) : value(t) {}
    template <typename T> any(storage_ref<T>* t) : value(t) {}

    template <typename OStream> void write(OStream& os) { value->write(os); }
    template <typename IStream> void read (IStream& is) { value->read (is); }
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

///////////////////////////////////////////////////////////
//
//                  Dimension system
//
///////////////////////////////////////////////////////////

// basic unit

template <int8_t Pow, int8_t Exp>
struct basic_unit
{
    static constexpr int8_t pow = Pow;
    static constexpr int8_t exp = Exp;
};

// pow result

template <int, typename>
struct pow_result_;

template <int P, typename U1>
using pow_result = typename pow_result_<P, U1>::type;

template <int P, int8_t Pow, int8_t Exp>
struct pow_result_<P, basic_unit<Pow, Exp>>
{
    using type = basic_unit<(P * Pow), Exp>;
};

template <int P, typename ...T>
struct pow_result_<P, sequence<T...>>
{
    using type = sequence<pow_result<P, T>...>;
};

// exp result

template <int, typename>
struct exp_result_;

template <int P, typename U1>
using exp_result = typename exp_result_<P, U1>::type;

template <int P, int8_t Pow, int8_t Exp>
struct exp_result_<P, basic_unit<Pow, Exp>>
{
    using type = basic_unit<Pow, P + Exp>;
};

template <int P, typename ...T>
struct exp_result_<P, sequence<T...>>
{
    using type = sequence<exp_result<P, T>...>;
};

// mul result

template <typename, typename>
struct mul_result_;

template <typename U0, typename U1>
using mul_result = typename mul_result_<U0, U1>::type;

template <int8_t Pow0, int8_t Pow1, int8_t Exp0, int8_t Exp1>
struct mul_result_<basic_unit<Pow0, Exp0>, basic_unit<Pow1, Exp1>>
{
    static_assert
    (
        (Exp0 == Exp1 || Pow0 == 0 || Pow1 == 0),
        "incorrect operands"
    );
    using type = basic_unit<Pow0 + Pow1, (Pow0 == 0 ? Exp0 : Exp1)>;
};

template <typename ...T0, typename ...T1>
struct mul_result_<sequence<T0...>, sequence<T1...>>
{
    using type = sequence<mul_result<T0, T1>...>;
};

// basic unit decimical conversion
// val * 10 ^ Exp1 => val * 10 ^ Exp0

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 > Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v / pow(10, Exp0 - Exp1);
}

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 < Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v * pow(10, Exp1 - Exp0);
}

template <typename ValueType, int8_t Exp0, int8_t Exp1>
inline typename std::enable_if<(Exp0 == Exp1), ValueType>::type
exp_convert(const ValueType& v)
{
    return v;
}

template <typename, typename, typename>
struct unit_convertor;

template <typename ValueType>
struct unit_convertor<ValueType, sequence<>, sequence<>>
{
    static ValueType convert(const ValueType& v) { return v; }
};

template
<
    typename ValueType,
    typename Head0, typename ...Tail0,
    typename Head1, typename ...Tail1
>
struct unit_convertor
<
    ValueType,
    sequence<Head0, Tail0...>, 
    sequence<Head1, Tail1...>
>
{
    static_assert
    (
        Head0::pow == Head1::pow,
        "incorrect phisical unit conversion"
    );

    static ValueType convert(const ValueType& v)
    {
        auto tmp = v;

        for(int8_t i = 0; i < Head0::pow; i++)
            tmp = exp_convert<ValueType, Head0::exp, Head1::exp>(tmp);

        return
        unit_convertor
        <
            ValueType,
            sequence<Tail0...>,
            sequence<Tail1...>
        >::convert(tmp);
    }
};

// phis value

template <typename ValueType, typename Unit>
class parameter
{
public:
    using value_type = ValueType;
    using unit_type = Unit;
private:
    value_type value;
public:
    parameter() : value(0) {}
    parameter(const value_type& v) : value(v) {}

    value_type get_value() const { return value; }
};

// parameter exp

template <int, typename>
struct dec_factor_;

template <int Exp, typename T>
using dec_factor = typename dec_factor_<Exp, T>::type;

template <int Exp, typename ValueType, typename Unit>
struct dec_factor_<Exp, parameter<ValueType, Unit>>
{
    using type = parameter<ValueType, exp_result<Exp, Unit>>;
};

// phis value cast

template <typename T0, typename T1>
T0 parameter_cast(const T1& p)
{
    using v_0 = typename T0::value_type;
    using u_0  = typename T0::unit_type;
    using u_1  = typename T1::unit_type;

    return T0(unit_convertor<v_0, u_0, u_1>::convert(p.get_value()));
}

// operators

template <typename V, typename U>
inline parameter<V, U>
operator + (const parameter<V, U>& p0, const parameter<V, U>& p1)
{
    parameter<V, U> res;
    res.value = p0.value + p1.value;
    return res;
}

template <typename V, typename U>
inline parameter<V, U>
operator - (const parameter<V, U>& p0, const parameter<V, U>& p1)
{
    parameter<V, U> res;
    res.value = p0.value - p1.value;
    return res;
}

template <typename V0, typename U0, typename V1, typename U1>
inline parameter<decltype(V0() * V1()), mul_result<U0, U1>>
operator * (const parameter<V0, U0>& p0, const parameter<V1, U1>& p1)
{
    return
    parameter
    <
        decltype(V0() * V1()),
        mul_result<U0, U1>
    >(p0.get_value() * p1.get_value());
}

template <typename V0, typename U0, typename V1, typename U1>
inline parameter<decltype(V0() / V1()), mul_result<U0, pow_result<-1, U1>>>
operator / (const parameter<V0, U0>& p0, const parameter<V1, U1>& p1)
{
    return
    parameter
    <
        decltype(V0() / V1()),
        mul_result<U0, pow_result<-1, U1>>
    >(p0.get_value() / p1.get_value());
}

// units

namespace basic_units {

template <int8_t ...P>
using unit = sequence<basic_unit<P, 0>...>;

using metre           = unit<1, 0, 0, 0, 0, 0, 0>;
using degree          = unit<0, 1, 0, 0, 0, 0, 0>;
using kilogram        = unit<0, 0, 1, 0, 0, 0, 0>;
using second          = unit<0, 0, 0, 1, 0, 0, 0>;
using kelvin          = unit<0, 0, 0, 0, 1, 0, 0>;
using ampere          = unit<0, 0, 0, 0, 0, 1, 0>;
using volt            = unit<0, 0, 0, 0, 0, 0, 1>;
using non_dimentional = unit<0, 0, 0, 0, 0, 0, 0>;

}

template <typename T> using metre    = parameter<T, basic_units::metre>;
template <typename T> using degree   = parameter<T, basic_units::degree>;
template <typename T> using kilogram = parameter<T, basic_units::kilogram>;
template <typename T> using second   = parameter<T, basic_units::second>;
template <typename T> using kelvin   = parameter<T, basic_units::kelvin>;
template <typename T> using ampere   = parameter<T, basic_units::ampere>;
template <typename T> using volt     = parameter<T, basic_units::volt>;

template <typename T>
using non_dimentional = parameter<T, basic_units::non_dimentional>;

}

#endif
