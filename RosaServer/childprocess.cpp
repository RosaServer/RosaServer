#include "childprocess.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>

ChildProcess::ChildProcess(std::string fileName) {
	if (pipe(fdParentToChild) == -1) {
		throw std::runtime_error(strerror(errno));
	}

	if (pipe(fdChildToParent) == -1) {
		close(fdParentToChild[0]);
		close(fdParentToChild[1]);

		throw std::runtime_error(strerror(errno));
	}

	pid = fork();

	if (pid == -1) {
		close(fdParentToChild[0]);
		close(fdParentToChild[1]);
		close(fdChildToParent[0]);
		close(fdChildToParent[1]);

		throw std::runtime_error(strerror(errno));
	}

	if (pid != 0) {
		close(fdParentToChild[0]);
		close(fdChildToParent[1]);

		fcntl(fdChildToParent[0], F_SETFL, O_NONBLOCK);
	} else {
		close(fdParentToChild[1]);
		close(fdChildToParent[0]);

		// Set reading the pipe to not block execution
		fcntl(fdParentToChild[0], F_SETFL, O_NONBLOCK);

		char strFromParentFD[10];
		char strToParentFD[10];

		sprintf(strFromParentFD, "%i", fdParentToChild[0]);
		sprintf(strToParentFD, "%i", fdChildToParent[1]);

		char* args[] = {(char*)"./rosaserversatellite", strFromParentFD,
		                strToParentFD, (char*)fileName.c_str(), nullptr};

		char workingDirectory[PATH_MAX];

		if (getcwd(workingDirectory, sizeof(workingDirectory)) == nullptr) {
			perror("getcwd");
			exit(1);
		}

		char ldPreload[PATH_MAX + 64];
		sprintf(ldPreload, "LD_PRELOAD=%s/libluajit.so", workingDirectory);

		char* env[] = {ldPreload, nullptr};

		if (execvpe(args[0], args, env) == -1) {
			perror("execvpe");
			exit(1);
		}
	}
}

ChildProcess::~ChildProcess() { terminate(); }

bool ChildProcess::isRunning() {
	if (gotExitCode || pid == -1) {
		return false;
	}

	int status;
	int retPID = waitpid(pid, &status, WNOHANG);

	if (retPID == -1) {
		throw std::runtime_error(strerror(errno));
	}

	if (retPID == 0) {
		return true;
	}

	gotExitCode = true;
	exitCode = status;
	return false;
}

void ChildProcess::terminate() {
	if (pid != -1) {
		if (kill(pid, SIGTERM) == -1) {
			if (errno != ESRCH) {
				throw std::runtime_error(strerror(errno));
			}
		} else {
			int status;
			int retPID = waitpid(pid, &status, 0);

			if (retPID == -1) {
				throw std::runtime_error(strerror(errno));
			}

			gotExitCode = true;
			exitCode = status;
		}

		// Close file handles
		close(fdParentToChild[1]);
		close(fdChildToParent[0]);

		pid = -1;
	}
}

sol::object ChildProcess::getExitCode(sol::this_state s) {
	sol::state_view lua(s);

	isRunning();

	if (gotExitCode) {
		return sol::make_object(lua, exitCode);
	}

	return sol::make_object(lua, sol::nil);
}

sol::object ChildProcess::receiveMessage(sol::this_state s) {
	sol::state_view lua(s);

	unsigned int length;

	auto bytesRead = read(fdChildToParent[0], &length, sizeof(length));
	if (bytesRead == -1) {
		if (errno != EAGAIN) {
			throw std::runtime_error(strerror(errno));
		}
	} else if (bytesRead == sizeof(length)) {
		std::string message(length, ' ');
		bytesRead = read(fdChildToParent[0], message.data(), length);
		if (bytesRead == -1) {
			if (errno != EAGAIN) {
				throw std::runtime_error(strerror(errno));
			}
		} else if (bytesRead == length) {
			return sol::make_object(lua, message);
		}
	}

	return sol::make_object(lua, sol::nil);
}

void ChildProcess::sendMessage(std::string message) {
	if (!isRunning()) return;

	unsigned int length = static_cast<unsigned int>(message.length());

	auto bytesWritten = write(fdParentToChild[1], &length, sizeof(length));
	if (bytesWritten == -1) {
		throw std::runtime_error(strerror(errno));
	} else if (bytesWritten != sizeof(length)) {
		throw std::runtime_error("Couldn't write full message to pipe");
	} else {
		bytesWritten = write(fdParentToChild[1], message.data(), length);
		if (bytesWritten == -1) {
			throw std::runtime_error(strerror(errno));
		} else if (bytesWritten != length) {
			throw std::runtime_error("Couldn't write full message to pipe");
		}
	}
}

void ChildProcess::setLimit(__rlimit_resource resource, rlim_t softLimit,
                            rlim_t hardLimit) {
	if (!isRunning()) return;

	const rlimit limits{softLimit, hardLimit};

	if (prlimit(pid, resource, &limits, nullptr) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}

void ChildProcess::setCPULimit(rlim_t softLimit, rlim_t hardLimit) {
	setLimit(RLIMIT_CPU, softLimit, hardLimit);
}

void ChildProcess::setMemoryLimit(rlim_t softLimit, rlim_t hardLimit) {
	setLimit(RLIMIT_AS, softLimit, hardLimit);
}

void ChildProcess::setFileSizeLimit(rlim_t softLimit, rlim_t hardLimit) {
	setLimit(RLIMIT_FSIZE, softLimit, hardLimit);
}

int ChildProcess::getPriority() {
	if (!isRunning()) return 0;

	// Necessary because a return value of -1 isnt always an error for
	// getpriority()
	errno = 0;

	int nice = getpriority(PRIO_PROCESS, pid);
	if (nice == -1 && errno != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return nice;
}

void ChildProcess::setPriority(int nice) {
	if (!isRunning()) return;

	if (setpriority(PRIO_PROCESS, pid, nice) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}