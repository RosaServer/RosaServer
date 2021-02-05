#pragma once
#include "sol/sol.hpp"

#include <sys/inotify.h>
#include <string>
#include <unordered_map>

static constexpr int bufferSize = sizeof(struct inotify_event) + NAME_MAX + 1;

class FileWatcher {
	int fd;
	std::unordered_map<int, std::string> watchDescriptors;
	char buffer[bufferSize]
	    __attribute__((aligned(__alignof__(struct inotify_event))));

 public:
	FileWatcher();
	~FileWatcher();
	void addWatch(const char* path, uint32_t mask);
	bool removeWatch(const char* path);
	sol::object receiveEvent(sol::this_state s);
};