#pragma once

#include <queue>
#include <string>
#include <mutex>

namespace Console
{
	extern std::queue<std::string> commandQueue;
	extern std::mutex commandQueueMutex;

	void threadMain();
	void init();
	void appendPrefix(std::string str);
	void log(std::string line);
}