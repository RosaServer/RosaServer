#include "worker.h"
#include "api.h"

#include <iostream>
#include <thread>

Worker::Worker()
{
}

Worker::~Worker()
{
	std::lock_guard<std::mutex> guard(destructionMutex);
	stop();
}

void Worker::runThread(const char* fileName)
{
	std::atomic_bool* _stopped = stopped;

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.open_libraries(sol::lib::package);
	lua.open_libraries(sol::lib::coroutine);
	lua.open_libraries(sol::lib::string);
	lua.open_libraries(sol::lib::os);
	lua.open_libraries(sol::lib::math);
	lua.open_libraries(sol::lib::table);
	lua.open_libraries(sol::lib::debug);
	lua.open_libraries(sol::lib::bit32);
	lua.open_libraries(sol::lib::io);
	lua.open_libraries(sol::lib::ffi);
	lua.open_libraries(sol::lib::jit);

	lua["os"]["realClock"] = l_os_clock;

	lua["sendMessage"] = [this](std::string message) {
		this->l_sendMessage(message);
	};

	lua["receiveMessage"] = [this](sol::this_state s) {
		return this->l_receiveMessage(s);
	};

	destructionMutex.lock();

	lua["sleep"] = [this, &_stopped](unsigned int ms) -> bool {
		// Allow this to be deconstructed while sleeping
		this->destructionMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));

		if (*_stopped) return true;

		this->destructionMutex.lock();
		return false;
	};

	{
		sol::load_result load = lua.load_file(fileName);
		if (noLuaCallError(&load))
		{
			sol::protected_function_result res = load();
			noLuaCallError(&res);
		}
	}

	if (!*_stopped)
	{
		destructionMutex.unlock();
	}

	while (!*_stopped)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	delete _stopped;
}

void Worker::l_sendMessage(std::string message)
{
	std::lock_guard<std::mutex> guard(receiveMessageQueueMutex);
	receiveMessageQueue.push(message);
	if (receiveMessageQueue.size() > 2047)
		receiveMessageQueue.pop();
}

sol::object Worker::l_receiveMessage(sol::this_state s)
{
	sol::state_view lua(s);

	sendMessageQueueMutex.lock();
	if (sendMessageQueue.empty())
	{
		sendMessageQueueMutex.unlock();
		return sol::make_object(lua, sol::lua_nil);
	}

	auto message = sendMessageQueue.front();
	sendMessageQueue.pop();
	sendMessageQueueMutex.unlock();
	return sol::make_object(lua, message);
}

void Worker::start(const char* fileName)
{
	if (!started && (!stopped || !*stopped))
	{
		started = true;
		stopped = new std::atomic_bool(false);

		std::thread thread(&Worker::runThread, this, fileName);
		thread.detach();
	}
}

void Worker::stop()
{
	if (stopped && !*stopped)
	{
		*stopped = true;
	}
}

void Worker::sendMessage(std::string message)
{
	if (!started || (stopped && *stopped)) return;

	std::lock_guard<std::mutex> guard(sendMessageQueueMutex);
	sendMessageQueue.push(message);
	if (sendMessageQueue.size() > 2047)
		sendMessageQueue.pop();
}

sol::object Worker::receiveMessage(sol::this_state s)
{
	sol::state_view lua(s);

	receiveMessageQueueMutex.lock();
	if (receiveMessageQueue.empty())
	{
		receiveMessageQueueMutex.unlock();
		return sol::make_object(lua, sol::lua_nil);
	}

	auto message = receiveMessageQueue.front();
	receiveMessageQueue.pop();
	receiveMessageQueueMutex.unlock();
	return sol::make_object(lua, message);
}