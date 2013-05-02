#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <array>
#include "common_protocol.h"

namespace robot
{

// log 2 for field size calculation

template <size_t Size>
struct log2 : std::integral_constant<uint8_t, log2<Size / 2>::value + 1>
{
    static_assert(Size > 1, "incorrect log arg");
};

template <>
struct log2<1> : std::integral_constant<uint8_t, 0> {};

///////////////////////////////////////////////////////////
//
//                          reg
//
///////////////////////////////////////////////////////////

namespace details
{
template <typename T>
struct reg_val_type_ : value_type_<T> {};

template <typename T, size_t C>
struct reg_val_type_<std::array<T, C>> : value_type_<T> {};
}

template <typename T>
struct reg_functions;

template <typename V, typename U>
struct reg_functions<phis_value<V, U>>
{
    using src = std::vector<V>;
    using type = phis_value<V, U>;

    static uint32_t field_count() { return 1; }

    static void read (const type& t, src& v) { v[0] = t.get_value(); }
    static void write(type& t, const src& v) { t.set_value(v[0]); }
};

template <typename V, typename U, size_t C>
struct reg_functions<std::array<phis_value<V, U>, C>>
{
    using src = std::vector<V>;
    using type = std::array<phis_value<V, U>, C>;

    static uint32_t field_count() { return C; }

    static void read(const type& t, src& v)
    {
        for(size_t i = 0; i < C; i++)
            v[i] = t[i].get_value();
    }

    static void write(type& t, const src& v)
    {
        for(size_t i = 0; i < C; i++)
            t[i].set_value(v[i]);
    }
};

template <typename T>
using reg_val_type = typename details::reg_val_type_<T>::type;

template
<
    typename T,
    uint8_t ACCESS_FLAGS,
    uint8_t READ_ACCESS_LEVEL = 0,
    uint8_t WRITE_P_ACCESS_LEVEL = 0,
    reg_val_type<T> MIN = std::numeric_limits<reg_val_type<T>>::min(),
    reg_val_type<T> MAX = std::numeric_limits<reg_val_type<T>>::max(),
    reg_val_type<T> STEP = 0
>
class reg
{
    static_assert(ACCESS_FLAGS != 0, "invalid reg access flags");

    using v_t = reg_val_type<T>;
    using p_t = parameter<ACCESS_FLAGS, v_t>;

    T data;

    signal on_update;

    std::mutex m;
    using lock_t = std::lock_guard<std::mutex>;

    static void check_vec_size(const std::vector<v_t>& v)
    {
        if(v.size() != reg_functions<T>::field_count())
            throw std::out_of_range("error: incorrect parameter size");
    }
public:
    void set(const T& t)
    {
        lock_t lock(m);
        data = t;
        on_update();
    }

    T get() const { return data; }

    void read_parameter_value(std::vector<v_t>& v)
    {
        check_vec_size(v);
        reg_functions<T>::read(data, v);
    }

    void write_parameter_value(const std::vector<v_t>& v)
    {
        check_vec_size(v);
        reg_functions<T>::write(data, v);
        on_update();
    }

    void add_action(const std::function<void()>& f) { on_update.add(f); }

    // binding with parameter

    std::shared_ptr<parameter_base> make_parameter(size_t p_code)
    {
        using p_t = parameter<ACCESS_FLAGS, v_t>;

        auto p = std::make_shared<p_t>(make_parameter_config(p_code));

        auto r = [this, p]() { this->read_parameter_value (p->val_ref()); };
        auto w = [this, p]() { this->write_parameter_value(p->val_ref()); };

        if(ACCESS_FLAGS & READ_FLAG)
            this->add_action(r);

        if(ACCESS_FLAGS & WRITE_FLAG)
            p->add_write_action(w);

        return p;
    }
private:
    parameter_config<reg_val_type<T>> make_parameter_config(size_t p_code)
    {
        return
        parameter_config<v_t>
        (
            parameter_access_config
            (
                ACCESS_FLAGS, // TODO check != 0
                p_code
            ),
            parameter_access_level_config
            (
                READ_ACCESS_LEVEL,
                WRITE_P_ACCESS_LEVEL
            ),
            parameter_value_type_config
            (
                reg_functions<T>::field_count(),
                log2<sizeof(v_t)>::value,
                (std::is_signed<v_t>::value ? 1 << 1 : 0) |
                (std::is_floating_point<v_t>::value ? 1 : 0)
            ),
            parameter_value_limits_config<v_t>
            (
                MAX,
                MIN,
                STEP
            ),
            repeat<uint8_t, uint8_t>()
        );
    }
};

}

#endif
