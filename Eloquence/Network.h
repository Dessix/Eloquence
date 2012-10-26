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
	std::set<SOCKET> connections;
	static int winsockRefCount;
public:
	NetworkInterface(Bot& bot) : bot(bot)
	{
		if(!Initialize())
		{
			throw std::exception("Winsock failed to initialize.");
		}
	}
	~NetworkInterface()
	{
		Cleanup();
	}

	bool Initialize()
	{
		assert(winsockRefCount>=0);
		if(winsockRefCount > 0)
		{
			++winsockRefCount;
			return true;
		} else {
			WSADATA wsadat;
			int res = WSAStartup(MAKEWORD(2,2), &wsadat);
			++winsockRefCount;
			return (res == 0);
		}
	}

	struct hostent *host;
	SOCKET Connect(const std::string& IP, uint32_t port)
	{
		assert(winsockRefCount>0);

		SOCKET sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(sock==INVALID_SOCKET)
		{
			throw std::exception("InvalidSocketException");
		}

		if((host=gethostbyname(IP.c_str()))==NULL)
		{
			throw std::exception("CouldNotResolveHostname");
		}

		SOCKADDR_IN SockAddr;
		SockAddr.sin_port=htons(port);
		SockAddr.sin_family=AF_INET;
		SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

		if(connect(sock,(SOCKADDR*)(&SockAddr),sizeof(SockAddr))!=0)
		{
			throw std::exception("InspecificSocketException");
		}

		u_long iMode = 1;
		ioctlsocket(sock, FIONBIO, &iMode);
		return sock;
	}
	void Disconnect(SOCKET sock)
	{
		assert(winsockRefCount>0);
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
			if(inDataLength <= 0)
			{
				break;
			}
			data += std::string(reinterpret_cast<const char*>(buffer), reinterpret_cast<const char*>(buffer)+inDataLength);
		}

		return data;
	}

	void Write(SOCKET sock, const std::string& data)
	{
		send(sock, data.c_str(), data.length(), 0);
	}

	void Cleanup()
	{
		assert(winsockRefCount > 0);
		std::for_each(connections.begin(), connections.end(), [this](SOCKET sock){
			this->Disconnect(sock);
		});
		if(winsockRefCount > 1)
		{
			--winsockRefCount;
		} else {
			WSACleanup();
		}
	}

	class SockHolder
	{
		NetworkInterface& net;
		bool closed;
		SOCKET sock;
	public:
		SockHolder(NetworkInterface& net, SOCKET sock) : net(net), sock(sock), closed(false)
		{}
		std::string Read()
		{
			if(closed)
			{
				throw std::exception("SocketClosed");
			}
			return net.Read(sock);
		}
		void Write(const std::string& data)
		{
			if(closed)
			{
				throw std::exception("SocketClosed");
			}
			net.Write(sock, data);
		}
		void Disconnect()
		{
			if(closed)
			{
				throw std::exception("SocketClosed");
			}
			closed = true;
			net.Disconnect(sock);
		}
	};

	SockHolder* CreateSocket(const std::string& IP, int Port)
	{
		SOCKET sock = Connect(IP, Port);
		return new SockHolder(*this, sock);
	}
};
