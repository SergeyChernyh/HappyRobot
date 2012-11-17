#include <algorithm>
#include <iostream>
#include "connection.h"

using namespace robot;

class test_socket
{
    char buf[100];
public:
    int read (char* dst, size_t n)
    {
        size_t p = n > 100 ? 100 : n;

        std::copy(buf, buf + p, dst);

        return p;
    }

    int write(const char* src, size_t n)
    {
        size_t p = n > 100 ? 100 : n;

        std::copy(src, src + p, buf);

        print();

        return p;
    }

    void print()
    {
        for(size_t i = 0; i < 100; i++)
            std::cout << std::hex << (unsigned)buf[i] << " ";
        std::cout << std::endl;
    }
};

int main()
{
    connection::connection p = test_socket();

    p.write(1, 3 , char(0x22), 8ull);

    metaprogramming::sequence<uint64_t, uint64_t, uint8_t> q;

    p.read(q);

    std::cout << at_c<0>(q) << " " << at_c<1>(q) << " " << (uint32_t)at_c<2>(q) << std::endl;

    return 0;
}
