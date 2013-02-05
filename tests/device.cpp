#include <iostream>
#include <sstream>

#include "device.h"

using namespace robot;

int main()
{
    reg<second<uint32_t>, READ_FLAG | WRITE_FLAG> r;
    reg<std::array<second<uint32_t>, 10>, READ_FLAG | WRITE_FLAG> arr;

    auto p = arr.make_parameter(0);

    auto writer = p->get_value_writer();

    std::stringstream s;
    s << "1 2 3 44 5 6 777    8  9 0";

    std::stringstream s0(s.str());

    std::array<second<uint32_t>, 10> tmp;

    for(size_t i = 0; i < 10; i++)
        s0 >> tmp[i];

    s >> writer;

    p->set_write();
    p->on_write();

    for(size_t i = 0; i < 10; i++)
        std::cout << arr.get()[i] << " ";
    std::cout << std::endl;

    arr.set(tmp);

    return 0;
}
