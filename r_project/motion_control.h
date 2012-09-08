#ifndef __MOTION_CONTROL__
#define __MOTION_CONTROL__

#include "subsystems/sonar_subsystem.h"
#include "subsystems/move_subsystem.h"

#include <vector>
#include <list>

namespace robot { namespace motion_control_details {

template <typename SizeType>
struct point
{
    SizeType x;
    SizeType y;
    SizeType z;
};

template <typename PointSizeType, typename SizeType, typename TimeType>
void update_point(point<PointSizeType>& point, subsystem::sonar<SizeType, TimeType>& p)
{
    using namespace subsystem;

    auto set_all_dimensions = [&]()
    {
        auto range = at_key<value>(p).get();

        auto x_p = at_key<x_pos>(p).get();
        auto y_p = at_key<y_pos>(p).get();
        auto z_p = at_key<z_pos>(p).get();

        auto x_v = at_key<x_vect_pos>(p).get();
        auto y_v = at_key<y_vect_pos>(p).get();
        auto z_v = at_key<z_vect_pos>(p).get();

        auto hyp_vect = sqrt((x_v * x_v + y_v * y_v + z_v * z_v).get());

        point.x = x_p + SizeType(range * x_v.get() / hyp_vect);
        point.y = y_p + SizeType(range * y_v.get() / hyp_vect);
        point.z = z_p + SizeType(range * z_v.get() / hyp_vect);

        //std::cout << point.x << " " << point.y << " " << point.z << '\n';
    };

    at_key<value>(p).add_effector(set_all_dimensions);
}

}

template <typename MoveSubsystem, typename SizeType>
class motion_control
{
    enum
    {
        MOVE_FREE,
        STOP,
        TRY_TURN_LEFT,
        TRY_TURN_RIGT
    } state;

    using point_t = motion_control_details::point<SizeType>;

    point_t tmp;
    std::list<point_t> points;
public:
    motion_control(MoveSubsystem& m):
        state(MOVE_FREE)
    {
    }

    template <typename T>
    void add_point(T& t)
    {
        points.push_back(motion_control_details::point<SizeType>());
        motion_control_details::update_point(points.back(), t);
    }
};

}

#endif //__MOTION_CONTROL__
