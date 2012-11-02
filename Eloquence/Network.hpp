#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <set>
#include <string>
#include <stdint.h>


class Bot;
class NetworkInterface
{
	Bot& bot;
	std::set<SOCKET> connections;
	static int winsockRefCount;
public:
	NetworkInterface(Bot& bot);
	~NetworkInterface();

	bool Initialize();

	SOCKET Connect(const std::string& IP, uint32_t port);
	SOCKET Connect(const std::string& IP);//Default Port version, port 80
	void Disconnect(SOCKET sock);

	std::string Read(SOCKET sock);

	void Write(SOCKET sock, const std::string& data);

	void Cleanup();

	class SockHolder
	{
		NetworkInterface& net;
		bool closed;
		SOCKET sock;
	public:
		SockHolder(NetworkInterface& net, SOCKET sock);
		~SockHolder();
		std::string Read();
		void Write(const std::string& data);
		void Disconnect();
	};

	SockHolder* CreateSocket(const std::string& IP, int Port);
};
