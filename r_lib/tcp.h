#ifndef __TCP__
#define __TCP__

#include <stdint.h>

#ifdef __WINDOWS__
    #include <winsock2.h>
    #include <Windows.h>
    #include <Ws2tcpip.h>

    enum { SHUTDOWN_OPT = SD_BOTH };
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>

    enum { SHUTDOWN_OPT = SHUT_RDWR };
#endif

namespace robot
{

class tcp_socket
{
    int io_socket;
public:
    tcp_socket(int s): io_socket(s) {}

    int write(const char* write_buffer, size_t size)
    {
        return send(io_socket, write_buffer, size, 0);
    }

    int read (char* read_buffer, size_t size)
    {
        return recv(io_socket, read_buffer, size, MSG_WAITALL);
    }
};

inline void tcp_init()
{
#ifdef __WINDOWS
    // magic
    WSAData dt;
    WSAStartup(0x0202, &dt);
#endif
}

inline tcp_socket tcp_client(uint32_t ip, uint16_t port)
{
    tcp_init();
    int io_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;

    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    int res = connect(io_socket, (sockaddr*)&addr, sizeof(addr));
    if(res)
        std::cerr << "tcp connect error\n"; // TODO exc

    return tcp_socket(io_socket);
}

inline tcp_socket wait_for_tcp_connection(uint32_t ip, uint16_t port)
{
    int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listener_socket < 0)
        std::cerr << "tcp listen error\n"; // TODO exc

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listener_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        std::cerr << "tcp bind error\n"; // TODO exc

    listen(listener_socket, 1);

    int io_socket = accept(listener_socket, 0, 0);

    if(io_socket < 0)
        std::cerr << "tcp accept error\n"; // TODO exc

    if(shutdown(listener_socket, SHUTDOWN_OPT) < 0)
        std::cerr << "tcp shutdown error";

#ifdef __WINDOWS__
    closesocket(listener_socket);
#else
    close(listener_socket);
#endif

    return tcp_socket(io_socket);
}

// only for tests!!!
namespace tcp_test
{

class TCPInterface
{
    typedef int socket_t;
    uint32_t ip_addr;
    uint16_t port;
    bool is_ready;

    socket_t io_socket;
    socket_t listener_socket;
    
    sockaddr_in my_addr, client_addr;

    int tcp_connect()
    {
        io_socket = socket(AF_INET, SOCK_STREAM, 0);
        my_addr.sin_addr.s_addr = ip_addr;
        my_addr.sin_port = htons(port);
        my_addr.sin_family = AF_INET;

        if(!connect(io_socket,(sockaddr*)&my_addr, sizeof(my_addr)))
            return 0;

        return 1;
    }

public:
    TCPInterface(uint16_t p = 8101):
        ip_addr(htonl(INADDR_LOOPBACK)),
        port(p),
        is_ready(false)
    {}
    void client_connect ()
    {
#ifdef __WINDOWS__
        WSAData dt;
        WSAStartup(0x0202, &dt);
#endif
        tcp_connect();
        is_ready = true;
    }

    void server_start()
    {
        listener_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(listener_socket < 0)
            return;
    
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(5200);
        client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(listener_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
            return;

        listen(listener_socket, 1);

        io_socket = accept(listener_socket, 0, 0);
        if(io_socket < 0)
            return;

        is_ready = true;
    }

    uint32_t write(const char* write_buffer, int length)
    {
        if(!is_ready)
            return -2;
        
        uint32_t send_count = send(io_socket, (char*)write_buffer, length, 0);

        if(!(int)send_count)
            return -1;
        
        return send_count;
    }

    uint32_t read (uint8_t* read_buffer,  int length)
    {
        if(!is_ready) {
            return -2;
        }

        int bytesRW = 0;

        if (length > 0) bytesRW = recv(io_socket,(char*)read_buffer,length,0);
        if(!(int)bytesRW)
        {
            return -1;
        }
        return bytesRW;
    }
};

}

}

#endif
