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
	int getMaxBytesPerSecond() const { return *Engine::serverMaxBytesPerSecond; }
	void setMaxBytesPerSecond(int max) const {
		*Engine::serverMaxBytesPerSecond = max;
	}
	char* getAdminPassword() const { return Engine::adminPassword; }
	void setAdminPassword(const char* newPassword) const {
		strncpy(Engine::adminPassword, newPassword, 31);
	}
	char* getPassword() const { return Engine::password; }
	void setPassword(const char* newPassword) const {
		strncpy(Engine::password, newPassword, 31);
		*Engine::isPassworded = newPassword[0] != 0;
	}
	int getMaxPlayers() const { return *Engine::maxPlayers; }
	void setMaxPlayers(int max) const { *Engine::maxPlayers = max; }

	int getWorldTraffic() const { return *Engine::World::traffic; }
	void setWorldTraffic(int traffic) const { *Engine::World::traffic = traffic; }
	int getWorldStartCash() const { return *Engine::World::startCash; }
	void setWorldStartCash(int cash) const { *Engine::World::startCash = cash; }
	int getWorldMinCash() const { return *Engine::World::minCash; }
	void setWorldMinCash(int cash) const { *Engine::World::minCash = cash; }
	bool getWorldShowJoinExit() const { return *Engine::World::showJoinExit; }
	void setWorldShowJoinExit(bool showJoinExit) const {
		*Engine::World::showJoinExit = showJoinExit;
	}
	bool getWorldRespawnTeam() const { return *Engine::World::respawnTeam; }
	void setWorldRespawnTeam(bool respawnTeam) const {
		*Engine::World::respawnTeam = respawnTeam;
	}

	int getWorldCrimeCivCiv() const { return *Engine::World::Crime::civCiv; }
	void setWorldCrimeCivCiv(int crime) const {
		*Engine::World::Crime::civCiv = crime;
	}
	int getWorldCrimeCivTeam() const { return *Engine::World::Crime::civTeam; }
	void setWorldCrimeCivTeam(int crime) const {
		*Engine::World::Crime::civTeam = crime;
	}
	int getWorldCrimeTeamCiv() const { return *Engine::World::Crime::teamCiv; }
	void setWorldCrimeTeamCiv(int crime) const {
		*Engine::World::Crime::teamCiv = crime;
	}
	int getWorldCrimeTeamTeam() const { return *Engine::World::Crime::teamTeam; }
	void setWorldCrimeTeamTeam(int crime) const {
		*Engine::World::Crime::teamTeam = crime;
	}
	int getWorldCrimeTeamTeamInBase() const {
		return *Engine::World::Crime::teamTeamInBase;
	}
	void setWorldCrimeTeamTeamInBase(int crime) const {
		*Engine::World::Crime::teamTeamInBase = crime;
	}
	int getWorldCrimeNoSpawn() const { return *Engine::World::Crime::noSpawn; }
	void setWorldCrimeNoSpawn(int crime) const {
		*Engine::World::Crime::noSpawn = crime;
	}

	int getRoundRoundTime() const { return *Engine::Round::roundTime; }
	void setRoundRoundTime(int minutes) const {
		*Engine::Round::roundTime = minutes;
	}
	int getRoundStartCash() const { return *Engine::Round::startCash; }
	void setRoundStartCash(int cash) const { *Engine::Round::startCash = cash; }
	bool getRoundIsWeekly() const { return *Engine::Round::weekly; }
	void setRoundIsWeekly(bool weekly) const { *Engine::Round::weekly = weekly; }
	bool getRoundHasBonusRatio() const { return *Engine::Round::bonusRatio; }
	void setRoundHasBonusRatio(bool ratio) const {
		*Engine::Round::bonusRatio = ratio;
	}
	int getRoundWeekDay() const { return *Engine::Round::weekDay; }
	void setRoundWeekDay(int day) const { *Engine::Round::weekDay = day; }
	int getRoundTeamDamage() const { return *Engine::Round::teamDamage; }
	void setRoundTeamDamage(int damage) const {
		*Engine::Round::teamDamage = damage;
	}

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

	void setConsoleTitle(const char* title) const { Console::setTitle(title); }
	void reset() const { hookAndReset(RESET_REASON_LUACALL); }
};
