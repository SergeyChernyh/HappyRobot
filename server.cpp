#include "device.h"
#include "tcp.h"

int main()
{
    using namespace robot;

    auto tcp = wait_for_tcp_connection(INADDR_LOOPBACK, 8101);
    server test_server(tcp);

    reg<second<uint32_t>, READ_FLAG | WRITE_FLAG> r;
    test_server.parameter_ref(1,0,16) = r.make_parameter(16);

    while(1)
        test_server.server_package_parse();

    return 0;
}
