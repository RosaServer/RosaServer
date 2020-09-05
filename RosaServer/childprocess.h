#pragma once
#include "sol.hpp"

#include <string>

class ChildProcess
{
	int fdParentToChild[2];
	int fdChildToParent[2];
	int pid;

	bool gotExitCode = false;
	int exitCode;
public:
	ChildProcess(std::string fileName);
	~ChildProcess();
	bool isRunning();
	void terminate();
	sol::object getExitCode(sol::this_state s);
	sol::object receiveMessage(sol::this_state s);
	void sendMessage(std::string message);
};