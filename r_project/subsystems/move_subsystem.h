#ifndef __MOVE_SUBSYSTEM__
#define __MOVE_SUBSYSTEM__

#include "parameter.h"

namespace robot { namespace subsystem {

    template <typename Key, typename T>
    using pair = metaprogramming::pair<Key, T>;

    struct abs_x;
    struct abs_y;
    struct abs_th;

    struct rel_x;
    struct rel_y;
    struct rel_th;

    struct linear_vel;
    struct angular_vel;

    struct linear_acc;
    struct angular_acc;

    struct max_linear_vel;
    struct max_angular_vel;

    struct max_linear_acc;
    struct max_angular_acc;

    struct set_linear_vel;
    struct set_angular_vel;

    struct set_linear_acc;
    struct set_angular_acc;

    struct move_to_abs_xy;
    struct move_to_rel_xy;

    struct rotate_abs;
    struct rotate_rel;
    struct rotate_current;

    struct move;
    struct stop;

    template <typename LengthUnit, typename AngleUnit, typename TimeUnit>
    using move_subsystem =
    subsystem
    <
        pair<abs_x, parameter<LengthUnit>>,
        pair<abs_y, parameter<LengthUnit>>,
        pair<abs_th, parameter<AngleUnit>>,

        pair<rel_x, parameter<LengthUnit>>,
        pair<rel_y, parameter<LengthUnit>>,
        pair<rel_th, parameter<AngleUnit>>,

        pair<linear_vel, parameter<decltype(LengthUnit()/TimeUnit())>>,
        pair<angular_vel, parameter<decltype(AngleUnit()/TimeUnit())>>,

        pair<linear_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,
        pair<angular_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,

        pair<max_linear_vel, parameter<decltype(LengthUnit()/TimeUnit())>>,
        pair<max_angular_vel, parameter<decltype(AngleUnit()/TimeUnit())>>,

        pair<max_linear_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,
        pair<max_angular_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,

        pair<set_linear_vel, parameter<decltype(LengthUnit()/TimeUnit())>>,
        pair<set_angular_vel, parameter<decltype(AngleUnit()/TimeUnit())>>,

        pair<set_linear_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,
        pair<set_angular_acc, parameter<decltype(LengthUnit()/TimeUnit()/TimeUnit())>>,

        pair<rotate_abs, parameter<bool>>,
        pair<rotate_rel, parameter<bool>>,
        pair<rotate_current, parameter<bool>>,

        pair<move, parameter<bool>>,
        pair<stop, parameter<bool>>
    >;
}}

#endif
