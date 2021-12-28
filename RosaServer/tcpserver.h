#pragma once

#include <memory>
#include <string>
#include <vector>

#include "sol/sol.hpp"

class TCPServerConnection {
	int socketDescriptor;
	uint16_t port;
	std::string address;

 public:
	TCPServerConnection(int socketDescriptor, uint16_t port, std::string address)
	    : socketDescriptor(socketDescriptor), port(port), address(address) {}
	~TCPServerConnection();

	void close();
	ssize_t send(std::string_view data) const;
	sol::object receive(sol::this_state state);

	bool isOpen() const { return socketDescriptor != -1; }
	uint16_t getPort() const { return port; }
	std::string getAddress() const { return address; }

	friend class TCPServer;
};

class TCPServer {
	int socketDescriptor;
	std::vector<std::shared_ptr<TCPServerConnection>> connections;

	void closeConnections();
	void clearClosedConnections();

 public:
	TCPServer(unsigned short port);
	~TCPServer();

	void close();
	sol::object accept(sol::this_state s);

	bool isOpen() const { return socketDescriptor != -1; }
};