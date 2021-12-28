#include "tcpserver.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

static constexpr int listenBacklog = 128;
static constexpr size_t maxReadSize = 4096;

static constexpr const char* errorNotOpen = "Socket is not open";

static inline void throwSafe() {
	char error[256];
	throw std::runtime_error(strerror_r(errno, error, sizeof(error)));
}

void TCPServerConnection::close() {
	if (socketDescriptor == -1) {
		throw std::runtime_error(errorNotOpen);
	}

	::close(socketDescriptor);
	socketDescriptor = -1;
}

TCPServerConnection::~TCPServerConnection() {
	if (socketDescriptor != -1) {
		close();
	}
}

ssize_t TCPServerConnection::send(std::string_view data) const {
	if (socketDescriptor == -1) {
		throw std::runtime_error(errorNotOpen);
	}

	if (data.empty()) {
		throw std::runtime_error("Data is empty");
	}

	auto bytesWritten = write(socketDescriptor, data.data(), data.size());
	if (bytesWritten == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return 0;
		}
		throwSafe();
	}

	return bytesWritten;
}

sol::object TCPServerConnection::receive(sol::this_state s) {
	if (socketDescriptor == -1) {
		throw std::runtime_error(errorNotOpen);
	}

	sol::state_view lua(s);

	char buffer[maxReadSize];
	auto bytesRead = read(socketDescriptor, buffer, maxReadSize);
	if (bytesRead == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return sol::make_object(lua, sol::nil);
		}
		throwSafe();
	}

	if (bytesRead == 0) {
		close();
	}

	std::string data(buffer, bytesRead);
	return sol::make_object(lua, data);
}

TCPServer::TCPServer(unsigned short port) {
	socketDescriptor = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socketDescriptor == -1) {
		throwSafe();
	}

	{
		int reuseAddress = 1;
		if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
		               sizeof(reuseAddress)) == -1) {
			throwSafe();
		}
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(socketDescriptor, reinterpret_cast<sockaddr*>(&serverAddress),
	         sizeof(serverAddress)) == -1) {
		throwSafe();
	}

	if (listen(socketDescriptor, listenBacklog) == -1) {
		throwSafe();
	}
}

void TCPServer::closeConnections() {
	for (auto& connection : connections) {
		if (connection->socketDescriptor != -1) {
			connection->close();
		}
	}
	connections.clear();
}

void TCPServer::clearClosedConnections() {
	auto it = connections.begin();
	while (it != connections.end()) {
		if ((*it)->socketDescriptor == -1) {
			it = connections.erase(it);
		} else {
			++it;
		}
	}
}

void TCPServer::close() {
	closeConnections();

	if (socketDescriptor == -1) {
		throw std::runtime_error(errorNotOpen);
	}

	::close(socketDescriptor);
	socketDescriptor = -1;
}

TCPServer::~TCPServer() {
	if (socketDescriptor != -1) {
		close();
	}
}

sol::object TCPServer::accept(sol::this_state s) {
	if (socketDescriptor == -1) {
		throw std::runtime_error(errorNotOpen);
	}

	sol::state_view lua(s);

	sockaddr_in address;
	socklen_t addressLength = sizeof(address);

	int clientDescriptor =
	    accept4(socketDescriptor, reinterpret_cast<sockaddr*>(&address),
	            &addressLength, SOCK_NONBLOCK);
	if (clientDescriptor == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return sol::make_object(lua, sol::nil);
		}
		throwSafe();
	}

	char addressString[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &address.sin_addr, addressString, INET_ADDRSTRLEN);

	auto connection = std::make_shared<TCPServerConnection>(
	    clientDescriptor, ntohs(address.sin_port), std::string(addressString));

	clearClosedConnections();
	connections.push_back(connection);
	return sol::make_object(lua, connection);
}