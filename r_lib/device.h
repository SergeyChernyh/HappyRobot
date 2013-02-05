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

class reg_value_signal
{
protected:
    signal on_update;
public:
    void add_action(const std::function<void()>& f) { on_update.add(f); }
};

template <typename T>
using reg_val_type = typename details::reg_val_type_<T>::type;

template <typename T, reg_val_type<T>, reg_val_type<T>, reg_val_type<T>>
struct reg_value;

template <typename V, typename U, V MIN, V MAX, V STEP>
struct reg_value<phis_value<V, U>, MIN, MAX, STEP>: public reg_value_signal
{
    using type = phis_value<V, U>;
    type data;

    std::mutex m;
    using lock_t = std::lock_guard<std::mutex>;
protected:
    uint32_t field_count() const { return 1; }
public:
    void set(const type& t)
    {
        lock_t lock(m);
        data = t;
        on_update();
    }

    void set(const V& v)
    {
        lock_t lock(m);
        data.set_value(v);
        on_update();
    }

    type get() const { return data; }

    void read_parameter_value(std::vector<V>& v)
    {
        if(v.size() != 1)
            throw std::out_of_range("error: incorrect parameter read size");

        v[0] = data.get_value();
    }

    void write_parameter_value(const std::vector<V>& v)
    {
        if(v.size() != 1)
            throw std::out_of_range("error: incorrect parameter read size");

        data.set_value(v[0]);
        on_update();
    }
};

template <typename V, typename U, size_t C, V MIN, V MAX, V STEP>
struct reg_value<std::array<phis_value<V, U>, C>, MIN, MAX, STEP>:
public reg_value_signal
{
    using type = std::array<phis_value<V, U>, C>;
    type data;

    std::mutex m;
    using lock_t = std::lock_guard<std::mutex>;
protected:
    uint32_t field_count() const { return C; }
public:
    void set(const type& t)
    {
        lock_t lock(m);
        data = t;
        on_update();
    }

    void set(const std::array<V, C>& v)
    {
        lock_t lock(m);
        for(size_t i = 0; i < C; i++)
            data[i].set_value(v[i]);
        on_update();
    }

    type get() const { return data; }

    void read_parameter_value(std::vector<V>& v)
    {
        if(v.size() != C)
            throw std::out_of_range("error: incorrect parameter read size");

        for(size_t i = 0; i < C; i++)
            v[i] = data[i].get_value();
    }

    void write_parameter_value(const std::vector<V>& v)
    {
        if(v.size() != C)
            throw std::out_of_range("error: incorrect parameter read size");

        for(size_t i = 0; i < C; i++)
            data[i].set_value(v[i]);

        on_update();
    }
};

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
class reg: public reg_value<T, MIN, MAX, STEP>
{
    static_assert(ACCESS_FLAGS != 0, "invalid reg access flags");

    using v_t = reg_val_type<T>;
    using p_t = parameter<ACCESS_FLAGS, v_t>;

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
                this->field_count(),
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
public:
    std::shared_ptr<parameter_base> make_parameter(size_t p_code)
    {
        using v_t = reg_val_type<T>;
        using p_t = parameter<ACCESS_FLAGS, v_t>;

        auto p = std::make_shared<p_t>(make_parameter_config(p_code));

        p_t* param = dynamic_cast<p_t*>(p.get());

        if(ACCESS_FLAGS & READ_FLAG) {
            this->add_action
            (
                [this, param]()
                {
                    this->read_parameter_value(param->val_ref());
                }
            );
        }

        if(ACCESS_FLAGS & WRITE_FLAG) {
            param->add_write_action
            (
                [this, param]()
                {
                    this->write_parameter_value(param->val_ref());
                }
            );
        }

        return p;
    }
};

}

#endif
