#ifndef __MOTION_CONTROL__
#define __MOTION_CONTROL__

#include "sonar_subsystem.h"
#include "move_subsystem.h"

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

        point.x = x_p + range * SizeType(x_v.get() / sqrt((x_v * x_v + y_v * y_v + z_v * z_v).get()));
        point.y = y_p + range * SizeType(y_v.get() / sqrt((x_v * x_v + y_v * y_v + z_v * z_v).get()));
        point.z = z_p + range * SizeType(z_v.get() / sqrt((x_v * x_v + y_v * y_v + z_v * z_v).get()));

        std::cout << point.x << " " << point.y << " " << point.z << '\n';
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
        std::cout << points.size() << "\n";
    }
};

/*
    void bind_subsystems()
    {
        auto& sonar2 = at_key<subsystem::value>(sonar_bar[2]);
        auto& sonar3 = at_key<subsystem::value>(sonar_bar[3]);
        auto& sonar4 = at_key<subsystem::value>(sonar_bar[4]);
        auto& sonar5 = at_key<subsystem::value>(sonar_bar[5]);

        auto checker = [&, this]()
        {
            if(sonar2.get() < 300 || sonar3.get() < 500 || sonar4.get() < 500 || sonar5.get() < 300) {
                this->execute_cmd<11>((int16_t)0);

                if(sonar2.get() + sonar3.get() > sonar4.get() + sonar5.get())
                    this->execute_cmd<21>((int16_t)(15));
                else
                    this->execute_cmd<21>((int16_t)(-15));
            }
            else {
                this->execute_cmd<11>((int16_t)(at_key<subsystem::set_linear_vel> (this->move_ctrl).get()));
                this->execute_cmd<21>((int16_t)(at_key<subsystem::set_angular_vel>(this->move_ctrl).get()));
            }
        };

        sonar3.add_effector(checker);
        sonar4.add_effector(checker);

        sonar2.add_effector(checker);
        sonar5.add_effector(checker);
    }
*/

}

#endif //__MOTION_CONTROL__
