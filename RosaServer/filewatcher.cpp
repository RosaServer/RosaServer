#include "filewatcher.h"

#include <cerrno>

FileWatcher::FileWatcher() {
	fd = inotify_init1(IN_NONBLOCK);
	if (fd < 0) {
		throw std::runtime_error(strerror(errno));
	}
}

FileWatcher::~FileWatcher() { close(fd); }

void FileWatcher::addWatch(const char* path, uint32_t mask) {
	int descriptor = inotify_add_watch(fd, path, mask);
	if (descriptor < 0) {
		throw std::runtime_error(strerror(errno));
	}
	watchDescriptors.emplace(descriptor, path);
}

bool FileWatcher::removeWatch(const char* path) {
	auto foundDescriptor =
	    std::find_if(std::begin(watchDescriptors), std::end(watchDescriptors),
	                 [&](const std::pair<int, std::string>& pair) {
		                 return pair.second == path;
	                 });
	if (foundDescriptor == std::end(watchDescriptors)) {
		return false;
	}

	if (inotify_rm_watch(fd, foundDescriptor->first) < 0) {
		throw std::runtime_error(strerror(errno));
	}

	watchDescriptors.erase(foundDescriptor->first);
	return true;
}

sol::object FileWatcher::receiveEvent(sol::this_state s) {
	sol::state_view lua(s);

	auto bytesRead = read(fd, buffer, sizeof(buffer));

	if (bytesRead < 0) {
		if (errno != EAGAIN) {
			throw std::runtime_error(strerror(errno));
		}

		return sol::make_object(lua, sol::nil);
	}

	const struct inotify_event* event =
	    reinterpret_cast<const struct inotify_event*>(buffer);

	sol::table table = lua.create_table();
	table["descriptor"] = watchDescriptors.at(event->wd);
	table["mask"] = event->mask;
	table["name"] = std::string(event->name);
	return sol::make_object(lua, table);
}