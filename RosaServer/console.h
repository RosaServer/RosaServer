#pragma once

#include <csignal>
#include <mutex>
#include <queue>
#include <string>

namespace Console {
extern std::queue<std::string> commandQueue;
extern std::mutex commandQueueMutex;
extern std::mutex autoCompleteMutex;
extern std::sig_atomic_t shouldExit;

bool isAwaitingAutoComplete();
std::string getAutoCompleteInput();
void respondToAutoComplete(std::string newBuffer);
void threadMain();
void init();
void cleanup();
void log(std::string line);
void handleInterruptSignal(int signal);
void setTitle(const char* title);
}  // namespace Console