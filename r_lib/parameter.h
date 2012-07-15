#ifndef __R_PARAMETER__
#define __R_PARAMETER__

#include <vector>
#include "dimension_table.h"
#include "package.h"

namespace robot { namespace common_protocol {
 
    template <typename T>
    class effector
    {
        struct lambda_wrap
        {
            auto static lambda = [](const T& t){};
        };

        using lambda_f = decltype(lambda_wrap::lambda);
   
        std::vector<lambda_f> vec;

    public:
        void add(const lambda_f& l) { vec.push_back(l); }

        template <typename A>
        void add(const A& a) { vec.push_back([=](const T& t){ a(t); }); }

        void operator() (const T& t)
        {
            for(const lambda_f& f : vec)
                f(t);
        }
    };
}}

#endif //__R_PARAMETER__
