#pragma once
#include "sol/sol.hpp"

#include <sys/resource.h>
#include <string>

class ChildProcess {
	int fdParentToChild[2];
	int fdChildToParent[2];
	int pid;

	bool gotExitCode = false;
	int exitCode;

	void setLimit(__rlimit_resource resource, rlim_t softLimit, rlim_t hardLimit);

 public:
	ChildProcess(const char* fileName);
	~ChildProcess();
	bool isRunning();
	void terminate();
	sol::object getExitCode(sol::this_state s);
	sol::object receiveMessage(sol::this_state s);
	void sendMessage(std::string_view message);
	void setCPULimit(rlim_t softLimit, rlim_t hardLimit);
	void setMemoryLimit(rlim_t softLimit, rlim_t hardLimit);
	void setFileSizeLimit(rlim_t softLimit, rlim_t hardLimit);
	int getPriority();
	void setPriority(int nice);
};