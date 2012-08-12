#ifndef __R_PARAMETER__
#define __R_PARAMETER__

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include "phis_value.h"

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

    template <typename T>
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
        void add_effector(const A& t)
        {
            act.add(t);
        }

        template <typename A>
        void set(const A& v)
        {
            lock_t lock(m);
            value = v;
            act();
        }

        T get() const
        {
            return value;
        }
    };

    template <typename ...Args>
    using subsystem = metaprogramming::sequence<Args...>;

    template <typename Key, typename T>
    using pair = metaprogramming::pair<Key, T>;
}}

#endif //__R_PARAMETER__
