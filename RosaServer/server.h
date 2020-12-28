#include <iostream>

#include "engine.h"

struct Server {
	const int TPS = 60;

	const char* getClass() const { return "Server"; }
	int getPort() const { return *Engine::serverPort; }
	char* getName() const { return Engine::serverName; }
	void setName(const char* newName) const {
		strncpy(Engine::serverName, newName, 31);
	}
	char* getPassword() const { return Engine::password; }
	void setPassword(const char* newPassword) const {
		strncpy(Engine::password, newPassword, 31);
		*Engine::isPassworded = newPassword[0] != 0;
	}
	int getMaxPlayers() const { return *Engine::maxPlayers; }
	void setMaxPlayers(int max) const { *Engine::maxPlayers = max; }
	int getType() const { return *Engine::gameType; }
	void setType(int type) const { *Engine::gameType = type; }
	char* getLevelName() const { return Engine::mapName; }
	void setLevelName(const char* newName) const {
		strncpy(Engine::mapName, newName, 31);
	}
	char* getLoadedLevelName() const { return Engine::loadedMapName; }
	bool getIsLevelLoaded() const { return *Engine::isLevelLoaded; }
	void setIsLevelLoaded(bool b) const { *Engine::isLevelLoaded = b; }
	float getGravity() const { return *Engine::gravity; }
	void setGravity(float gravity) const { *Engine::gravity = gravity; }
	float getDefaultGravity() const { return Engine::originalGravity; }
	int getState() const { return *Engine::gameState; }
	void setState(int state) const { *Engine::gameState = state; }
	int getTime() const { return *Engine::gameTimer; }
	void setTime(int time) const { *Engine::gameTimer = time; }
	int getSunTime() const { return *Engine::sunTime; }
	void setSunTime(int time) const { *Engine::sunTime = time % 5184000; }
	std::string getVersion() const {
		std::ostringstream stream;
		stream << *Engine::version << (char)(*Engine::subVersion + 'a');
		return stream.str();
	}
	unsigned int getVersionMajor() const { return *Engine::version; }
	unsigned int getVersionMinor() const { return *Engine::subVersion; }
	unsigned int getNumEvents() const { return *Engine::numEvents; }

	void setConsoleTitle(const char* title) const { Console::setTitle(title); }
	void reset() const { hookAndReset(RESET_REASON_LUACALL); }
};