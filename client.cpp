#include <string>
#include "common_protocol.h"
#include "tcp.h"

int main()
{
    using namespace robot;

    auto tcp = tcp_client(INADDR_LOOPBACK, 5200);
    client test_client(tcp);

    test_client.update_config();

    while(1) {
        std::cout << ">";
        std::string cmd_name;
        std::cin >> cmd_name;

        if(cmd_name == "set") {
            test_client.set_parameter_values(std::cin);
        }
        else if(cmd_name == "write") {
            test_client.write_parameter_values(std::cin);
        }
        else if(cmd_name == "read") {
            ;
        }
        else if(cmd_name == "exit") {
            break; 
        }
        else
            std::cout << "no such command\n";
    }

    return 0;
}
