#include "sol/sol.hpp"

#include <chrono>
#include <thread>
#include <unistd.h>

static int fdFromParent;
static int fdToParent;

static double l_os_clock()
{
	auto now = std::chrono::steady_clock::now();
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto epoch = ms.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	return value.count() / 1000.;
}

static sol::object l_receiveMessage(sol::this_state s)
{
	sol::state_view lua(s);

	unsigned int length;

	auto bytesRead = read(fdFromParent, &length, sizeof(length));
	if (bytesRead == -1)
	{
		if (errno != EAGAIN)
		{
			throw std::runtime_error(strerror(errno));
		}
	}
	else if (bytesRead == sizeof(length))
	{
		std::string message(length, ' ');
		bytesRead = read(fdFromParent, message.data(), length);
		if (bytesRead == -1)
		{
			if (errno != EAGAIN)
			{
				throw std::runtime_error(strerror(errno));
			}
		}
		else if (bytesRead == length)
		{
			return sol::make_object(lua, message);
		}
	}

	return sol::make_object(lua, sol::lua_nil);
}

static void l_sendMessage(std::string message)
{
	unsigned int length = static_cast<unsigned int>(message.length());

	auto bytesWritten = write(fdToParent, &length, sizeof(length));
	if (bytesWritten == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	else if (bytesWritten != sizeof(length))
	{
		throw std::runtime_error("Couldn't write full message to pipe");
	}
	else
	{
		bytesWritten = write(fdToParent, message.data(), length);
		if (bytesWritten == -1)
		{
			throw std::runtime_error(strerror(errno));
		}
		else if (bytesWritten != length)
		{
			throw std::runtime_error("Couldn't write full message to pipe");
		}
	}
}

int main(int argc, const char* argv[])
{
	if (argc < 4) return 1;

	fdFromParent = atoi(argv[1]);
	fdToParent = atoi(argv[2]);
	const char* fileName = argv[3];

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

	lua["receiveMessage"] = [](sol::this_state s) {
		return l_receiveMessage(s);
	};

	lua["sendMessage"] = [](std::string message) {
		l_sendMessage(message);
	};

	lua["sleep"] = [](unsigned int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	};

	sol::load_result load = lua.load_file(fileName);
	if (load.valid())
	{
		sol::protected_function_result res = load();
		if (!res.valid())
		{
			return 3;
		}
	}
	else
	{
		return 2;
	}

	return 0;
}