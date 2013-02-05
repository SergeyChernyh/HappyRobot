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
    connection p = test_socket();

    sequence<int, int, char, uint64_t> seq(1, 2, 0x22, 8);

    p.write(seq);

    sequence<uint64_t, uint64_t, uint8_t> q;

    p.read(q);

    std::cout << get<0>(q) << " " << get<1>(q) << " " << (uint32_t)get<2>(q) << std::endl;

    return 0;
}
