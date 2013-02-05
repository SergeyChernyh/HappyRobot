#include "common_protocol.h"
#include "tcp.h"

int main()
{
    using namespace robot;

    auto tcp = tcp_client(INADDR_LOOPBACK, 8101);
    client test_client(tcp);

    test_client.update_config();

    char hz;
    tcp.read(&hz, 1);

    return 0;
}
