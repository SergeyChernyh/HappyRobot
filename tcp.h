#ifndef __TCP__
#define __TCP__

#include <stdint.h>

#include <Windows.h>

namespace robot
{
namespace tcp_test
{

class TCPInterface
{
	uint16_t wsa_version;
	uint32_t ip_addr;
	uint16_t port;
	bool is_ready;

	SOCKET _socket;
	
	sockaddr_in _my_addr, _client_addr; // _client_addr - теперь, кажется, не нужен

	int tcp_connect()
	{
		_socket = socket(AF_INET,SOCK_STREAM,0);
		_my_addr.sin_addr.s_addr = ip_addr;
		_my_addr.sin_port = htons(port);
		_my_addr.sin_family = AF_INET;

		while(connect(_socket,(sockaddr*)&_my_addr, sizeof(_my_addr))==SOCKET_ERROR)
		{
			//printf("0_o %i\n", GetLastError());
			Sleep(1); // HACK - чтобы сильно не грузило. Лучше не придумал
		}

		return 0;
	}

public:
	TCPInterface():
	wsa_version(0x0202),
	ip_addr(inet_addr("127.0.0.1")),
	port(8101),
	is_ready(false)
	{}
	void open ()
	{
		WSAData dt;
		WSAStartup(wsa_version,&dt);
		//printf("WSA_ver %i, error %i\n",wsa_version, GetLastError());
		tcp_connect();
		is_ready = true;
	}

	uint32_t write(const char* write_buffer, int length)
	{
		if(!is_ready)
			return -2;
		
		uint32_t send_count = send(_socket, (char*)write_buffer, length, 0);

		if((int)send_count == SOCKET_ERROR)
			return -1;
		
		return send_count;
	}

	uint32_t read (uint8_t* read_buffer,  int length)
	{
		if(!is_ready) return -2;
		DWORD bytesRW = 0;

		if (length > 0) bytesRW = recv(_socket,(char*)read_buffer,length,0);
		if((int)bytesRW==SOCKET_ERROR)
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
