#ifndef __TCP__
#define __TCP__

#include <stdint.h>

#ifdef __WINDOWS__
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

namespace robot
{

#ifdef __WINDOWS__
inline void delay( unsigned long ms )
{
    Sleep(ms);
}
#else
#include <unistd.h>

inline void delay( unsigned long ms )
{
    usleep(ms * 1000);
}
#endif

namespace tcp_test
{

class TCPInterface
{
#ifdef __WINDOWS__
    uint16_t wsa_version;
    typedef SOCKET socket_t;
#else
    typedef int socket_t;
#endif
    uint32_t ip_addr;
    uint16_t port;
    bool is_ready;

    socket_t _socket;
    
    sockaddr_in _my_addr, _client_addr;

    int tcp_connect()
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        _my_addr.sin_addr.s_addr = ip_addr;
        _my_addr.sin_port = htons(port);
        _my_addr.sin_family = AF_INET;

        if(!connect(_socket,(sockaddr*)&_my_addr, sizeof(_my_addr)))
            return 0;

        return 1;
    }

public:
    TCPInterface():
#ifdef __WINDOWS__
    wsa_version(0x0202),
    ip_addr(inet_addr("127.0.0.1")),
#else
    ip_addr(htonl(INADDR_LOOPBACK)),
#endif
    port(8101),
    is_ready(false)
    {}
    void open ()
    {
#ifdef __WINDOWS__
        WSAData dt;
        WSAStartup(wsa_version,&dt);
#endif
        tcp_connect();
        is_ready = true;
    }

    uint32_t write(const char* write_buffer, int length)
    {
        if(!is_ready)
            return -2;
        
        uint32_t send_count = send(_socket, (char*)write_buffer, length, 0);

        if(!(int)send_count)
            return -1;
        
        return send_count;
    }

    uint32_t read (uint8_t* read_buffer,  int length)
    {
        if(!is_ready) return -2;
        int bytesRW = 0;

        if (length > 0) bytesRW = recv(_socket,(char*)read_buffer,length,0);
        if(!(int)bytesRW)
        {
            //printf("SOCKET_ERROR reading %i\n", GetLastError());
            return -1;
        }
        return bytesRW;
    }
};

}

}

#endif
