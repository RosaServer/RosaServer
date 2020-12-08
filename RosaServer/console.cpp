// Used 'tinycon' by unix-ninja as reference
// https://github.com/unix-ninja/hackersandbox/blob/master/tinycon.cpp

#include "console.h"

#include <termios.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <thread>

namespace Console {
std::queue<std::string> commandQueue;
std::mutex commandQueueMutex;
std::mutex autoCompleteMutex;
std::sig_atomic_t shouldExit = false;

// Allows getting characters before newline and disables echo
static int _getch() {
	struct termios oldMode, newMode;

	tcgetattr(STDIN_FILENO, &oldMode);
	newMode = oldMode;
	newMode.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newMode);

	int code = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldMode);

	return code;
}

static constexpr int CODE_TAB = 9;
static constexpr int CODE_NEW_LINE = 10;
static constexpr int CODE_ESCAPE = 27;
static constexpr int CODE_1 = 49;
static constexpr int CODE_3 = 51;
static constexpr int CODE_4 = 52;
static constexpr int CODE_A = 65;
static constexpr int CODE_B = 66;
static constexpr int CODE_C = 67;
static constexpr int CODE_D = 68;
static constexpr int CODE_LEFT_BRACKET = 91;
static constexpr int CODE_TILDE = 126;
static constexpr int CODE_BACKSPACE = 127;

static std::mutex outputMutex;

static constexpr int historyLimit = 512;
static std::deque<std::string> history;
static int historyPos = -1;

static std::string draft;
static std::deque<char> buffer;
static int cursorCol = 0;

static bool inputInitialized = false;

static std::string getBuffer() {
	return std::string(buffer.begin(), buffer.end());
}

static void redrawLine() {
	std::cout << "\033[36;1m>\033[0m ";
	std::cout << getBuffer();

	int numBack = buffer.size() - cursorCol;
	for (int i = 0; i < numBack; i++) {
		std::cout << '\b';
	}

	std::cout << std::flush;
}

static bool awaitingAutoComplete = false;
static std::string autoCompleteInput;

bool isAwaitingAutoComplete() {
	std::lock_guard<std::mutex> guard(autoCompleteMutex);
	return awaitingAutoComplete;
}

std::string getAutoCompleteInput() {
	std::lock_guard<std::mutex> guard(autoCompleteMutex);
	return autoCompleteInput;
}

void respondToAutoComplete(std::string newBuffer) {
	std::lock_guard<std::mutex> guard(autoCompleteMutex);
	if (!awaitingAutoComplete) {
		return;
	}

	awaitingAutoComplete = false;

	buffer.clear();
	for (int i = 0; i < newBuffer.size(); i++) {
		buffer.push_back(newBuffer[i]);
	}

	for (int i = 0; i < cursorCol; i++) {
		std::cout << "\b \b";
	}

	cursorCol = buffer.size();

	std::lock_guard<std::mutex> outputGuard(outputMutex);
	std::cout << newBuffer;
}

static void awaitAutoComplete() {
	{
		std::lock_guard<std::mutex> guard(autoCompleteMutex);
		awaitingAutoComplete = true;
		autoCompleteInput = getBuffer();
	}

	while (true) {
		{
			std::lock_guard<std::mutex> guard(autoCompleteMutex);
			if (!awaitingAutoComplete) {
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(32));
	}
}

void threadMain() {
	int retryCode = 0;

	while (true) {
		int code;
		if (retryCode) {
			code = retryCode;
			retryCode = 0;
		} else {
			code = _getch();
		}

		switch (code) {
			case CODE_TAB:
				awaitAutoComplete();
				break;

			case CODE_NEW_LINE:
				if (buffer.size()) {
					std::string bufferString = getBuffer();

					if (!history.size() || history[0] != bufferString) {
						history.push_front(bufferString);

						if (history.size() > historyLimit) {
							history.pop_back();
						}
					}

					buffer.clear();
					cursorCol = 0;
					historyPos = -1;

					log("\033[32;1m>\033[0m " + bufferString + "\n");

					std::lock_guard<std::mutex> guard(commandQueueMutex);
					commandQueue.push(bufferString);
				}
				break;

			case CODE_BACKSPACE: {
				if (cursorCol == 0) break;

				std::lock_guard<std::mutex> guard(outputMutex);

				std::cout << "\b \b";

				if (cursorCol == buffer.size()) {
					buffer.pop_back();
					cursorCol--;
				} else {
					cursorCol--;
					buffer.erase(buffer.begin() + cursorCol);

					for (int i = cursorCol; i < buffer.size(); i++) {
						std::cout << buffer[i];
					}

					std::cout << ' ';

					for (int i = cursorCol + 1; i < buffer.size(); i++) {
						std::cout << '\b';
					}

					std::cout << "\b\b";
				}
			} break;

			// Parse escape sequences
			case CODE_ESCAPE:
				code = _getch();
				if (code == CODE_LEFT_BRACKET) {
					code = _getch();
					switch (code) {
						// A = up arrow
						case CODE_A:
							if (history.size()) {
								if (historyPos == -1) {
									draft = "";
									draft.assign(buffer.begin(), buffer.end());
								}

								std::lock_guard<std::mutex> guard(outputMutex);

								for (int i = 0; i < cursorCol; i++) {
									std::cout << "\b \b";
								}

								buffer.clear();

								historyPos++;
								if (historyPos > history.size() - 1) {
									historyPos = history.size() - 1;
								}

								auto historyLine = history[historyPos];

								for (int i = 0; i < historyLine.size(); i++) {
									buffer.push_back(historyLine[i]);
								}

								cursorCol = buffer.size();
								std::cout << historyLine;
							}
							break;

						// B = down arrow
						case CODE_B:
							if (history.size()) {
								std::lock_guard<std::mutex> guard(outputMutex);

								for (int i = 0; i < cursorCol; i++) {
									std::cout << "\b \b";
								}

								buffer.clear();

								historyPos--;
								if (historyPos < -1) {
									historyPos = -1;
								}

								if (historyPos >= 0) {
									auto historyLine = history[historyPos];

									std::cout << historyLine;

									for (int i = 0; i < historyLine.size(); i++) {
										buffer.push_back(historyLine[i]);
									}
								} else {
									std::cout << draft;
									for (int i = 0; i < draft.size(); i++) {
										buffer.push_back(draft[i]);
									}
								}

								cursorCol = buffer.size();
							}
							break;

						// C = right arrow
						case CODE_C:
							if (cursorCol < buffer.size()) {
								std::lock_guard<std::mutex> guard(outputMutex);
								std::cout << buffer[cursorCol];
								cursorCol++;
							}
							break;

						// D = left arrow
						case CODE_D:
							if (cursorCol) {
								std::lock_guard<std::mutex> guard(outputMutex);
								std::cout << '\b';
								cursorCol--;
							}
							break;

						// 1~ = home
						case CODE_1:
						// 3~ = delete
						case CODE_3:
						// 4~ = end
						case CODE_4: {
							int secondCode = _getch();
							if (secondCode == CODE_TILDE) {
								if (code == CODE_1) {
									if (cursorCol > 0) {
										std::lock_guard<std::mutex> guard(outputMutex);
										for (int i = 0; i < cursorCol; i++) {
											std::cout << '\b';
										}
										cursorCol = 0;
									}
								} else if (code == CODE_3) {
									if (cursorCol < buffer.size()) {
										buffer.erase(buffer.begin() + cursorCol);

										std::lock_guard<std::mutex> guard(outputMutex);

										for (int i = cursorCol; i < buffer.size(); i++) {
											std::cout << buffer[i];
										}

										std::cout << ' ';

										for (int i = cursorCol; i < buffer.size() + 1; i++) {
											std::cout << '\b';
										}
									}
								} else if (code == CODE_4) {
									std::lock_guard<std::mutex> guard(outputMutex);

									for (int i = cursorCol; i < buffer.size(); i++) {
										std::cout << buffer[i];
									}
									cursorCol = buffer.size();
								}
							} else {
								retryCode = secondCode;
								continue;
							}
						} break;

						default:
							retryCode = code;
							continue;
					}
				} else {
					retryCode = code;
					continue;
				}
				break;

			default:
				if (code >= 0x20) {
					char character = static_cast<char>(code);

					std::lock_guard<std::mutex> guard(outputMutex);
					std::cout << character << std::flush;

					if (cursorCol == buffer.size()) {
						buffer.push_back(character);
					} else {
						buffer.insert(buffer.begin() + cursorCol, character);

						for (int i = cursorCol + 1; i < buffer.size(); i++) {
							std::cout << buffer[i];
						}

						for (int i = cursorCol + 1; i < buffer.size(); i++) {
							std::cout << '\b';
						}
					}

					cursorCol++;
				}
				break;
		}
	}
}

void init() {
	inputInitialized = true;

	std::thread thread(threadMain);
	thread.detach();
}

void cleanup() {
	struct termios mode;

	tcgetattr(STDIN_FILENO, &mode);
	mode.c_lflag |= (ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &mode);
}

void log(std::string line) {
	std::lock_guard<std::mutex> guard(outputMutex);

	// Erase current line, move cursor to start, print
	std::cout << "\33[2K\r";
	std::cout << line;

	if (inputInitialized && !shouldExit) redrawLine();
}

void handleInterruptSignal(int signal) { shouldExit = true; }

void setTitle(const char* title) {
	std::lock_guard<std::mutex> guard(outputMutex);
	std::cout << "\33]0;" << title << '\7';
}
}  // namespace Console