#include "common_protocol.h"

using namespace robot;

int main()
{
    parameter<3, uint16_t> p(parameter_config<uint16_t>(parameter_access_config(3, 0), parameter_access_level_config(0, 0), parameter_value_type_config(1, 1, 0)));

    p.on_read();
    p.on_write();

    return 0;
}
