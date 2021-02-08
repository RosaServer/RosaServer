#pragma once
#include "sol/sol.hpp"

#include <atomic>
#include <mutex>
#include <queue>
#include <string>

class Worker {
	bool* stopped = nullptr;
	std::mutex stoppedMutex;
	std::mutex destructionMutex;

	std::queue<std::string> sendMessageQueue;
	std::mutex sendMessageQueueMutex;

	std::queue<std::string> receiveMessageQueue;
	std::mutex receiveMessageQueueMutex;

	void runThread(std::string fileName);
	void l_sendMessage(std::string message);
	sol::object l_receiveMessage(sol::this_state s);

 public:
	Worker(std::string fileName);
	~Worker();
	void stop();
	void sendMessage(std::string message);
	sol::object receiveMessage(sol::this_state s);
};