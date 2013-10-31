#include <type_traits>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include "data_types.h"
#include "dimension.h"

using namespace robot;

struct char_key;
struct long_key;
struct sub_sequence_key;

using sequence_0_t = sequence<int, pair<char_key, char>,  pair<sub_sequence_key, sequence<short, pair<long_key, long>>>, int*>;

struct key0;
struct key1;
struct key2;

template <typename T0, typename T1>
using sequence_1_t =
sequence
<
    std::integral_constant<T0, 55>,
    std::integral_constant<T1, 44>,
    sequence
    <
    >,
    sequence
    <
        pair<key0, int>,
        pair<key1, char>,
        pair<key2, std::integral_constant<int, 99>>
    >
>;

using sequence_10_t = sequence_1_t<char, short>;

using is_compile_time_constant_check =
sequence
<
    pair<int, std::integral_constant<int, 55>>,
    std::integral_constant<short, 44>,
    sequence
    <
    >
>;

int main()
{
    ///////////////////////////////////////////////////////
    //
    //                  sequence test
    //
    ///////////////////////////////////////////////////////

    {

    static_assert(std::is_same<value_type_at_c<0, sequence_0_t>, int>::value, "0");
    static_assert(std::is_same<value_type_at_c<1, sequence_0_t>, char>::value, "1");
    static_assert(std::is_same<value_type_at_c<2, sequence_0_t>, sequence<short, pair<long_key, long>>>::value, "2");
    static_assert(std::is_same<value_type_at_c<3, sequence_0_t>, int*>::value, "3");

    sequence_0_t sequence_0;
    const sequence_0_t& sequence_0_o = sequence_0;

    get<0>(sequence_0_t());
    get<0>(sequence_0_o);

    get<0>(sequence_0) = 3;

    get<long_key>(get<2>(sequence_0)) = 71;

    static_assert(std::is_same<decltype (robot::sequence_element<unsigned char>::value), uint8_t >::value, "0");

    assert(get<1>(get<sub_sequence_key>(sequence_0)) == 71);
    assert(get<1>(get<sub_sequence_key>(sequence_0)) == get<long_key>(get<2>(sequence_0)));
    assert(&get<1>(get<sub_sequence_key>(sequence_0)) == &get<long_key>(get<2>(sequence_0)));

    sequence_0_t sequence_01(1, 2, sequence<short, pair<long_key, long>>(9, 431), (int*)(0));

    assert(get<0>(sequence_01) == 1);
    assert(get<1>(sequence_01) == 2);

    assert(get<1>(get<sub_sequence_key>(sequence_01)) == 431);
    assert(get<1>(get<sub_sequence_key>(sequence_01)) == get<long_key>(get<2>(sequence_01)));
    assert(&get<1>(get<sub_sequence_key>(sequence_01)) == &get<long_key>(get<2>(sequence_01)));

    sequence_10_t
    sequence_10 = sequence_10_t
    (
        sequence<int, int, std::integral_constant<int, 99>>(3, 4)
    );

    auto p = get<key1>(get<3>(sequence_10));
    static_assert(std::is_same<decltype(p), char>::value, "additional check");

    static_assert(is_constexpr<std::integral_constant<char, 55>>::value, "is_compile_time_constant error 0");
    static_assert(is_constexpr<is_compile_time_constant_check>::value, "is_compile_time_constant error 1");

    sequence_10_t
    sequence_101 = sequence_10_t
    (
        sequence<int, int, std::integral_constant<int, 99>>()
    );

    assert(get<1>(sequence_10) == 44);
    assert(get<1>(sequence_101) == 44);
    assert(get<2>(get<3>(sequence_101)) == 99);

    static_assert(std::is_same<value_type_at_c<0, sequence_10_t>, const char>::value, "0");

    int r0 = 0;
    char r1 = 1;
    short r2 = 2;

    sequence<const int&, const char&, const short&> tie(r0, r1, r2);

    get<0>(tie);

    }

    ///////////////////////////////////////////////////////
    //
    //               serialization test
    //
    ///////////////////////////////////////////////////////


    {
        size_calc_stream s;

        std::tuple<int, short, char, unsigned long long> a(0x00010203,0x0405,0x6,0x0708090a0b0c0d0e);
        std::tuple<char, short, int, int, int> b;

        s << a;

        assert(s.get() == 15);

        binary_buffer buf(s.get());
    
        binary_ostream os(buf);
        binary_istream is(buf);

        os << a;
        is >> b;

        assert(std::get<0>(b) == 0x3 && std::get<1>(b) == 0x0102);
    }

    {
    size_calc_stream s;

    int r0 = 0x01020304;

    repeat<unsigned char, short> vec;

    vec.push_back(0x0506);
    vec.push_back(0x0708);
    vec.push_back(0x0910);

    s << r0 << vec;

    assert(s.get() == 11);

    binary_buffer buf(s.get());
    
    binary_ostream os(buf);
    binary_istream is(buf);

    os << r0 << vec;

    char c0, c1, c2, c3;

    repeat<unsigned char, char> cvec;

    is >> c0 >> c1 >> c2 >> c3 >> cvec;

    assert(c0 == 4);
    assert(c1 == 3);
    assert(c2 == 2);
    assert(c3 == 1);

    assert(cvec.size() == 3);

    assert(cvec[0] == 6);
    assert(cvec[1] == 5);
    assert(cvec[2] == 8);

    sequence
    <
        sequence
        <
            sequence
            <
                std::integral_constant<int, 0x04030201>,
                unsigned short
            >,
            repeat<unsigned char, char>
        >
    >
    seq0
    (
        sequence
        <
            sequence
            <
                std::integral_constant<int, 0x04030201>,
                unsigned short
            >,
            repeat<unsigned char, char>
        >(0x0506, cvec)
    );
    
    size_calc_stream s_;

    s_ << seq0;


    binary_buffer buf_(s_.get());
    
    binary_ostream os_(buf_);
    binary_istream is_(buf_);

    os_ << seq0;

    sequence<char, short, int> seq1;

    is_ >> seq1;

    assert(s_.get() == 10);
    assert(get<0>(seq1) == 0x1);
    assert(get<1>(seq1) == 0x0302);
    assert(get<2>(seq1) == 0x03050604);

        
    size_calc_stream s__;

    s__ << seq0;


    binary_buffer buf__(s__.get());
    
    binary_ostream os__(buf__);
    binary_istream is__(buf__);

    get<1>(get<0>(get<0>(seq0))) = 0x0708;

    os__ << seq0;
    any hz = make_storage_ref(seq1);
    is__ >> hz;

    assert(s__.get() == 10);
    assert(get<0>(seq1) == 0x1);
    assert(get<1>(seq1) == 0x0302);
    assert(get<2>(seq1) == 0x03070804);
    }

    ///////////////////////////////////////////////////////
    //
    //                  Dimensions
    //
    ///////////////////////////////////////////////////////

    using hz = pow_result<-1, basic_units::second>;

    static_assert
    (
        std::is_same<hz, basic_units::unit<0, 0, 0, -1, 0, 0, 0>>::value,
        "Dimensions 0"
    );

    static_assert
    (
        std::is_same
        <
            mul_result<basic_units::metre, pow_result<-2, basic_units::second>>,
            basic_units::unit<1, 0, 0, -2, 0, 0, 0>
        >::value,
        "Dimensions 1"
    );

    metre<int> l = 10;
    second<int> t = 2;

    using sm = dec_factor<-2, metre<int>>;
    using ms = dec_factor<-1, second<int>>;

    sm lsm = phis_cast<sm>(l);

    assert(lsm == l);

    auto v = l / t;

    auto vmmsec = lsm / t;

    auto vsmmsec = phis_cast<decltype(sm() / ms())>(l / t);

    auto v1 = phis_cast<decltype(v)>(vmmsec);

    assert(v1.get_value() == 5);
    assert(vsmmsec.get_value() == 50);

    assert(lsm.get_value() == 1000);
    assert(v.get_value() == 5);
    assert(vmmsec.get_value() == 500);

    int a = 77;

    size_calc_stream size;

    size << a;

    binary_buffer _buf_(size.get());

    binary_ostream _os_(_buf_);
    binary_istream _is_(_buf_);

    _os_ << a;
    _is_ >> l;

    assert(l.get_value() == a);

    return 0;
}
