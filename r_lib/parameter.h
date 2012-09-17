#ifndef __R_PARAMETER__
#define __R_PARAMETER__

#include <vector>
#include <array>
#include <functional>
#include <thread>
#include <mutex>
#include <limits>
#include "phis_value.h"
#include "metaprogramming/type_traits.h"

namespace robot { namespace subsystem {
 
    class effector
    {
        using f_t = std::function<void()>;
        using f_vect_t = std::vector<f_t>;

        f_vect_t vec;

    public:
        template <typename A>
        void add(const A& a) { vec.push_back([=](){ a(); }); }

        void operator() ()
        {
            for(const f_t f : vec)
                f();
        }
    };

    namespace details
    {
        template <typename T>
        struct value_type_
        {
            using type = metaprogramming::value_type<T>;
        };

        template <typename T>
        using value_type = typename value_type_<T>::type;

        template <typename T, typename D>
        struct value_type_<phis_value<T, D>>
        {
            using type = value_type<T>;
        };

        template <typename T, size_t C>
        struct value_type_<std::array<T, C>>
        {
            using type = value_type<T>;
        };

        template <typename T>
        struct data_type_
        {
            using type = T;
        };

        template <typename T, typename D>
        struct data_type_<phis_value<T, D>>
        {
            using type = T;
        };

        template <typename T>
        using data_type = typename data_type_<T>::type;
    }

    struct NA;
    struct RO;
    struct WO;
    struct RW;

    template
    <
        typename T,
        typename Type = RW,
        uint8_t READ_LEVEL = 0,
        uint8_t WRITE_LEVEL = 0,
        details::value_type<T> MIN  = std::numeric_limits<details::value_type<T>>::min(),
        details::value_type<T> MAX  = std::numeric_limits<details::value_type<T>>::max(),
        details::value_type<T> STEP = 0
    >
    class parameter
    {
        T value;

        effector act;

        std::mutex m;

        using lock_t = std::lock_guard<std::mutex>;

    public:
        parameter() {}
        parameter(const T& v): value(v) {}

        template <typename A>
        void add_effector(const A& t) { act.add(t); }

        template <typename A>
        void set(const A& v)
        {
            lock_t lock(m);
            value = v;
            act();
        }

        T get() const { return value; }
    };

    template <typename ...Args>
    using subsystem = metaprogramming::sequence<Args...>;

    template <typename Key, typename T>
    using pair = metaprogramming::pair<Key, T>;
}}

#endif //__R_PARAMETER__
