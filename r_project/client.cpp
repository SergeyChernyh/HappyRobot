#include "tcp.h"
#include "client.h"

int main(int argc, const char* argv[])
{
    using namespace subsystem;

    tcp_test::TCPInterface tcp(5200);

    tcp.client_connect();

    console_client<tcp_test::TCPInterface> client(tcp);

    return 0;
}
