#ifndef __R_PARAMETER__
#define __R_PARAMETER__

#include <vector>
#include <functional>
#include <limits>
#include <thread>
#include <mutex>
#include "phis_value.h"
#include "package.h"

namespace robot { namespace subsystem {
 
    template <typename T>
    class effector
    {
        using f_t = std::function<void(const T&)>;
        using f_vect_t = std::vector<f_t>;

        f_vect_t vec;

    public:
        template <typename A>
        void add(const A& a) { vec.push_back([=](const T& t){ a(t); }); }

        void operator() (const T& t)
        {
            for(const f_t f : vec)
                f(t);
        }
    };

    template <typename T, T max = std::numeric_limits<T>::max(), T min = std::numeric_limits<T>::min(), T step = 1>
    class parameter
    {
        T value;

        effector<T> act;

        std::mutex m;

    public:
        parameter() {}
        parameter(const T& v): value(v) {}

        template <typename A>
        void add_effector(const A& t)
        {
            act.add(t);
        }

        void set(const T& v)
        {
            std::lock_guard<std::mutex> lock(m);
            value = v;
            act(v);
        }

        void get() const
        {
            std::lock_guard<std::mutex> lock(m);
            return value;
        }
    };

    template <typename ...Args>
    using subsystem = metaprogramming::sequence<Args...>;
}}

#endif //__R_PARAMETER__
