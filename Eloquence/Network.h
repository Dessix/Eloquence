#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <set>
#include <string>
#include <stdint.h>
#include <assert.h>
#include <algorithm>
#pragma comment (lib, "ws2_32.lib")

class Bot;
class NetworkInterface
{
	Bot& bot;
	bool initialized;
	std::set<SOCKET> connections;
public:
	NetworkInterface(Bot& bot) : bot(bot), initialized(false)
	{
	}

	bool Initialize()
	{
		assert(!initialized);
		WSADATA wsadat;
		int res = WSAStartup(MAKEWORD(2,2), &wsadat);
		return (res == 0);
	}

	struct hostent *host;
	SOCKET Connect(std::string IP, uint32_t port)
	{
		assert(initialized);

		SOCKET sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(sock==INVALID_SOCKET)
		{
			return 0;
		}

		if((host=gethostbyname(IP.c_str()))==NULL)
		{
			return 0;
		}

		SOCKADDR_IN SockAddr;
		SockAddr.sin_port=htons(port);
		SockAddr.sin_family=AF_INET;
		SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

		if(connect(sock,(SOCKADDR*)(&SockAddr),sizeof(SockAddr))!=0)
		{
			return 0;
		}

		u_long iMode = 1;
		ioctlsocket(sock, FIONBIO, &iMode);
		return sock;
	}
	void Disconnect(SOCKET sock)
	{
		assert(initialized);
		shutdown(sock, SD_BOTH);
		connections.erase(sock);
		closesocket(sock);
	}

	std::string Read(SOCKET sock)
	{
		std::string data("");
		char buffer[1001];
		ZeroMemory(buffer, 1001);
		while(true)
		{
			int inDataLength = recv(sock, buffer, 1000, 0);
			int nError = WSAGetLastError();
			if(nError != 0 && nError != WSAEWOULDBLOCK)
			{
				throw std::exception("InspecificSocketException");
			}
			if(inDataLength == 0)
			{
				break;
			}
			data += std::string(buffer, inDataLength);
		}

		return data;
	}

	void Write(SOCKET sock, std::string data)
	{
	}

	void Cleanup()
	{
		assert(initialized);
		std::for_each(connections.begin(), connections.end(), [this](SOCKET sock){
			this->Disconnect(sock);
		});
		WSACleanup();
	}
};
