#pragma once
#include "sol/sol.hpp"

#include <queue>
#include <atomic>
#include <mutex>
#include <string>

class Worker
{
	bool started = false;
	std::atomic_bool* stopped = nullptr;
	std::mutex destructionMutex;

	std::queue<std::string> sendMessageQueue;
	std::mutex sendMessageQueueMutex;

	std::queue<std::string> receiveMessageQueue;
	std::mutex receiveMessageQueueMutex;

	void runThread(const char* fileName);
	void l_sendMessage(std::string message);
	sol::object l_receiveMessage(sol::this_state s);
public:
	Worker();
	~Worker();
	void start(const char* fileName);
	void stop();
	void sendMessage(std::string message);
	sol::object receiveMessage(sol::this_state s);
};