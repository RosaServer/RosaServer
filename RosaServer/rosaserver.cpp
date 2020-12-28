#include "rosaserver.h"
#include <sys/mman.h>
#include <cerrno>

static void pryMemory(void* address, size_t numPages) {
	size_t pageSize = sysconf(_SC_PAGE_SIZE);

	uintptr_t page = (uintptr_t)address;
	page -= (page % pageSize);

	if (mprotect((void*)page, pageSize * numPages, PROT_WRITE | PROT_READ) == 0) {
		std::ostringstream stream;

		stream << RS_PREFIX "Successfully pried open page at ";
		stream << std::showbase << std::hex;
		stream << static_cast<uintptr_t>(page);
		stream << "\n";

		Console::log(stream.str());
	} else {
		throw std::runtime_error(strerror(errno));
	}
}

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
static Server* server;

void defineThreadSafeAPIs(sol::state* state) {
	state->open_libraries(sol::lib::base);
	state->open_libraries(sol::lib::package);
	state->open_libraries(sol::lib::coroutine);
	state->open_libraries(sol::lib::string);
	state->open_libraries(sol::lib::os);
	state->open_libraries(sol::lib::math);
	state->open_libraries(sol::lib::table);
	state->open_libraries(sol::lib::debug);
	state->open_libraries(sol::lib::bit32);
	state->open_libraries(sol::lib::io);
	state->open_libraries(sol::lib::ffi);
	state->open_libraries(sol::lib::jit);

	{
		auto meta = state->new_usertype<Vector>("new", sol::no_constructor);
		meta["x"] = &Vector::x;
		meta["y"] = &Vector::y;
		meta["z"] = &Vector::z;

		meta["class"] = sol::property(&Vector::getClass);
		meta["__tostring"] = &Vector::__tostring;
		meta["__add"] = &Vector::__add;
		meta["__sub"] = &Vector::__sub;
		meta["__mul"] = sol::overload(&Vector::__mul, &Vector::__mul_RotMatrix);
		meta["__div"] = &Vector::__div;
		meta["__unm"] = &Vector::__unm;
		meta["add"] = &Vector::add;
		meta["mult"] = &Vector::mult;
		meta["set"] = &Vector::set;
		meta["clone"] = &Vector::clone;
		meta["dist"] = &Vector::dist;
		meta["distSquare"] = &Vector::distSquare;
	}

	{
		auto meta = state->new_usertype<RotMatrix>("new", sol::no_constructor);
		meta["x1"] = &RotMatrix::x1;
		meta["y1"] = &RotMatrix::y1;
		meta["z1"] = &RotMatrix::z1;
		meta["x2"] = &RotMatrix::x2;
		meta["y2"] = &RotMatrix::y2;
		meta["z2"] = &RotMatrix::z2;
		meta["x3"] = &RotMatrix::x3;
		meta["y3"] = &RotMatrix::y3;
		meta["z3"] = &RotMatrix::z3;

		meta["class"] = sol::property(&RotMatrix::getClass);
		meta["__tostring"] = &RotMatrix::__tostring;
		meta["__mul"] = &RotMatrix::__mul;
		meta["set"] = &RotMatrix::set;
		meta["clone"] = &RotMatrix::clone;
	}

	{
		auto meta = state->new_usertype<Image>("Image");
		meta["width"] = sol::property(&Image::getWidth);
		meta["height"] = sol::property(&Image::getHeight);
		meta["numChannels"] = sol::property(&Image::getNumChannels);
		meta["free"] = &Image::_free;
		meta["loadFromFile"] = &Image::loadFromFile;
		meta["loadBlank"] = &Image::loadBlank;
		meta["getRGB"] = &Image::getRGB;
		meta["getRGBA"] = &Image::getRGBA;
		meta["setPixel"] = sol::overload(&Image::setRGB, &Image::setRGBA);
		meta["getPNG"] = &Image::getPNG;
	}

	(*state)["print"] = Lua::print;

	(*state)["Vector"] = sol::overload(Lua::Vector_, Lua::Vector_3f);
	(*state)["RotMatrix"] = Lua::RotMatrix_;

	(*state)["os"]["listDirectory"] = Lua::os::listDirectory;
	(*state)["os"]["createDirectory"] = Lua::os::createDirectory;
	(*state)["os"]["realClock"] = Lua::os::realClock;
	(*state)["os"]["exit"] = sol::overload(Lua::os::exit, Lua::os::exitCode);

	{
		auto httpTable = state->create_table();
		(*state)["http"] = httpTable;
		httpTable["getSync"] = Lua::http::getSync;
		httpTable["postSync"] = Lua::http::postSync;
	}
}

void luaInit(bool redo) {
	std::lock_guard<std::mutex> guard(stateResetMutex);

	if (redo) {
		Console::log(LUA_PREFIX "Resetting state...\n");
		delete server;

		for (int i = 0; i < maxNumberOfPlayers; i++) {
			if (playerDataTables[i]) {
				delete playerDataTables[i];
				playerDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfHumans; i++) {
			if (humanDataTables[i]) {
				delete humanDataTables[i];
				humanDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfItems; i++) {
			if (itemDataTables[i]) {
				delete itemDataTables[i];
				itemDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfVehicles; i++) {
			if (vehicleDataTables[i]) {
				delete vehicleDataTables[i];
				vehicleDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfRigidBodies; i++) {
			if (bodyDataTables[i]) {
				delete bodyDataTables[i];
				bodyDataTables[i] = nullptr;
			}
		}

		delete lua;
	} else {
		Console::log(LUA_PREFIX "Initializing state...\n");
	}

	lua = new sol::state();

	Console::log(LUA_PREFIX "Defining...\n");
	defineThreadSafeAPIs(lua);

	{
		auto meta = lua->new_usertype<Server>("new", sol::no_constructor);
		meta["TPS"] = &Server::TPS;

		meta["class"] = sol::property(&Server::getClass);
		meta["port"] = sol::property(&Server::getPort);
		meta["name"] = sol::property(&Server::getName, &Server::setName);
		meta["password"] =
		    sol::property(&Server::getPassword, &Server::setPassword);
		meta["maxPlayers"] =
		    sol::property(&Server::getMaxPlayers, &Server::setMaxPlayers);
		meta["type"] = sol::property(&Server::getType, &Server::setType);
		meta["levelToLoad"] =
		    sol::property(&Server::getLevelName, &Server::setLevelName);
		meta["loadedLevel"] = sol::property(&Server::getLoadedLevelName);
		meta["isLevelLoaded"] =
		    sol::property(&Server::getIsLevelLoaded, &Server::setIsLevelLoaded);
		meta["gravity"] = sol::property(&Server::getGravity, &Server::setGravity);
		meta["defaultGravity"] = sol::property(&Server::getDefaultGravity);
		meta["state"] = sol::property(&Server::getState, &Server::setState);
		meta["time"] = sol::property(&Server::getTime, &Server::setTime);
		meta["sunTime"] = sol::property(&Server::getSunTime, &Server::setSunTime);
		meta["version"] = sol::property(&Server::getVersion);
		meta["versionMajor"] = sol::property(&Server::getVersionMajor);
		meta["versionMinor"] = sol::property(&Server::getVersionMinor);
		meta["numEvents"] = sol::property(&Server::getNumEvents);

		meta["setConsoleTitle"] = &Server::setConsoleTitle;
		meta["reset"] = &Server::reset;
	}

	server = new Server();
	(*lua)["server"] = server;

	{
		auto meta = lua->new_usertype<Connection>("new", sol::no_constructor);
		meta["port"] = &Connection::port;
		meta["timeoutTime"] = &Connection::timeoutTime;

		meta["class"] = sol::property(&Connection::getClass);
		meta["address"] = sol::property(&Connection::getAddress);
		meta["adminVisible"] = sol::property(&Connection::getAdminVisible,
		                                     &Connection::setAdminVisible);
	}

	{
		auto meta = lua->new_usertype<Account>("new", sol::no_constructor);
		meta["subRosaID"] = &Account::subRosaID;
		meta["phoneNumber"] = &Account::phoneNumber;
		meta["money"] = &Account::money;
		meta["corporateRating"] = &Account::corporateRating;
		meta["criminalRating"] = &Account::criminalRating;
		meta["spawnTimer"] = &Account::spawnTimer;
		meta["playTime"] = &Account::playTime;
		meta["banTime"] = &Account::banTime;

		meta["class"] = sol::property(&Account::getClass);
		meta["__tostring"] = &Account::__tostring;
		meta["index"] = sol::property(&Account::getIndex);
		meta["name"] = sol::property(&Account::getName);
		meta["steamID"] = sol::property(&Account::getSteamID);
	}

	{
		auto meta = lua->new_usertype<Player>("new", sol::no_constructor);
		meta["subRosaID"] = &Player::subRosaID;
		meta["phoneNumber"] = &Player::phoneNumber;
		meta["money"] = &Player::money;
		meta["corporateRating"] = &Player::corporateRating;
		meta["criminalRating"] = &Player::criminalRating;
		meta["team"] = &Player::team;
		meta["teamSwitchTimer"] = &Player::teamSwitchTimer;
		meta["stocks"] = &Player::stocks;
		meta["spawnTimer"] = &Player::spawnTimer;
		meta["menuTab"] = &Player::menuTab;
		meta["numActions"] = &Player::numActions;
		meta["lastNumActions"] = &Player::lastNumActions;
		meta["numMenuButtons"] = &Player::numMenuButtons;
		meta["gender"] = &Player::gender;
		meta["skinColor"] = &Player::skinColor;
		meta["hairColor"] = &Player::hairColor;
		meta["hair"] = &Player::hair;
		meta["eyeColor"] = &Player::eyeColor;
		meta["model"] = &Player::model;
		meta["suitColor"] = &Player::suitColor;
		meta["tieColor"] = &Player::tieColor;
		meta["head"] = &Player::head;
		meta["necklace"] = &Player::necklace;

		meta["class"] = sol::property(&Player::getClass);
		meta["__tostring"] = &Player::__tostring;
		meta["index"] = sol::property(&Player::getIndex);
		meta["isActive"] =
		    sol::property(&Player::getIsActive, &Player::setIsActive);
		meta["data"] = sol::property(&Player::getDataTable);
		meta["name"] = sol::property(&Player::getName, &Player::setName);
		meta["isAdmin"] = sol::property(&Player::getIsAdmin, &Player::setIsAdmin);
		meta["isReady"] = sol::property(&Player::getIsReady, &Player::setIsReady);
		meta["isBot"] = sol::property(&Player::getIsBot, &Player::setIsBot);
		meta["human"] = sol::property(&Player::getHuman, &Player::setHuman);
		meta["connection"] = sol::property(&Player::getConnection);
		meta["account"] = sol::property(&Player::getAccount, &Player::setAccount);
		meta["botDestination"] =
		    sol::property(&Player::getBotDestination, &Player::setBotDestination);

		meta["getAction"] = &Player::getAction;
		meta["getMenuButton"] = &Player::getMenuButton;
		meta["update"] = &Player::update;
		meta["updateFinance"] = &Player::updateFinance;
		meta["remove"] = &Player::remove;
		meta["sendMessage"] = &Player::sendMessage;
	}

	{
		auto meta = lua->new_usertype<Human>("new", sol::no_constructor);
		meta["stamina"] = &Human::stamina;
		meta["maxStamina"] = &Human::maxStamina;
		meta["vehicleSeat"] = &Human::vehicleSeat;
		meta["despawnTime"] = &Human::despawnTime;
		meta["movementState"] = &Human::movementState;
		meta["zoomLevel"] = &Human::zoomLevel;
		meta["damage"] = &Human::damage;
		meta["pos"] = &Human::pos;
		meta["viewYaw"] = &Human::viewYaw;
		meta["viewPitch"] = &Human::viewPitch;
		meta["strafeInput"] = &Human::strafeInput;
		meta["walkInput"] = &Human::walkInput;
		meta["inputFlags"] = &Human::inputFlags;
		meta["lastInputFlags"] = &Human::lastInputFlags;
		meta["health"] = &Human::health;
		meta["bloodLevel"] = &Human::bloodLevel;
		meta["chestHP"] = &Human::chestHP;
		meta["headHP"] = &Human::headHP;
		meta["leftArmHP"] = &Human::leftArmHP;
		meta["rightArmHP"] = &Human::rightArmHP;
		meta["leftLegHP"] = &Human::leftLegHP;
		meta["rightLegHP"] = &Human::rightLegHP;
		meta["gender"] = &Human::gender;
		meta["head"] = &Human::head;
		meta["skinColor"] = &Human::skinColor;
		meta["hairColor"] = &Human::hairColor;
		meta["hair"] = &Human::hair;
		meta["eyeColor"] = &Human::eyeColor;
		meta["model"] = &Human::model;
		meta["suitColor"] = &Human::suitColor;
		meta["tieColor"] = &Human::tieColor;
		meta["necklace"] = &Human::necklace;

		meta["class"] = sol::property(&Human::getClass);
		meta["__tostring"] = &Human::__tostring;
		meta["index"] = sol::property(&Human::getIndex);
		meta["isActive"] = sol::property(&Human::getIsActive, &Human::setIsActive);
		meta["data"] = sol::property(&Human::getDataTable);
		meta["isAlive"] = sol::property(&Human::getIsAlive, &Human::setIsAlive);
		meta["isImmortal"] =
		    sol::property(&Human::getIsImmortal, &Human::setIsImmortal);
		meta["isOnGround"] = sol::property(&Human::getIsOnGround);
		meta["isStanding"] = sol::property(&Human::getIsStanding);
		meta["isBleeding"] =
		    sol::property(&Human::getIsBleeding, &Human::setIsBleeding);
		meta["player"] = sol::property(&Human::getPlayer, &Human::setPlayer);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);
		meta["rightHandItem"] = sol::property(&Human::getRightHandItem);
		meta["leftHandItem"] = sol::property(&Human::getLeftHandItem);
		meta["rightHandGrab"] =
		    sol::property(&Human::getRightHandGrab, &Human::setRightHandGrab);
		meta["leftHandGrab"] =
		    sol::property(&Human::getLeftHandGrab, &Human::setLeftHandGrab);
		meta["isAppearanceDirty"] = sol::property(&Human::getIsAppearanceDirty,
		                                          &Human::setIsAppearanceDirty);

		meta["remove"] = &Human::remove;
		meta["teleport"] = &Human::teleport;
		meta["speak"] = &Human::speak;
		meta["arm"] = &Human::arm;
		meta["getBone"] = &Human::getBone;
		meta["getRigidBody"] = &Human::getRigidBody;
		meta["setVelocity"] = &Human::setVelocity;
		meta["addVelocity"] = &Human::addVelocity;
		meta["mountItem"] = &Human::mountItem;
		meta["applyDamage"] = &Human::applyDamage;
	}

	{
		auto meta = lua->new_usertype<ItemType>("new", sol::no_constructor);
		meta["price"] = &ItemType::price;
		meta["mass"] = &ItemType::mass;
		meta["fireRate"] = &ItemType::fireRate;
		meta["bulletType"] = &ItemType::bulletType;
		meta["bulletVelocity"] = &ItemType::bulletVelocity;
		meta["bulletSpread"] = &ItemType::bulletSpread;
		meta["numHands"] = &ItemType::numHands;
		meta["rightHandPos"] = &ItemType::rightHandPos;
		meta["leftHandPos"] = &ItemType::leftHandPos;
		meta["boundsCenter"] = &ItemType::boundsCenter;

		meta["class"] = sol::property(&ItemType::getClass);
		meta["__tostring"] = &ItemType::__tostring;
		meta["index"] = sol::property(&ItemType::getIndex);
		meta["name"] = sol::property(&ItemType::getName, &ItemType::setName);
		meta["isGun"] = sol::property(&ItemType::getIsGun, &ItemType::setIsGun);
	}

	{
		auto meta = lua->new_usertype<Item>("new", sol::no_constructor);
		meta["physicsSettledTimer"] = &Item::physicsSettledTimer;
		meta["type"] = &Item::type;
		meta["despawnTime"] = &Item::despawnTime;
		meta["parentSlot"] = &Item::parentSlot;
		meta["pos"] = &Item::pos;
		meta["vel"] = &Item::vel;
		meta["rot"] = &Item::rot;
		meta["bullets"] = &Item::bullets;
		meta["computerCurrentLine"] = &Item::computerCurrentLine;
		meta["computerTopLine"] = &Item::computerTopLine;
		meta["computerCursor"] = &Item::computerCursor;

		meta["class"] = sol::property(&Item::getClass);
		meta["__tostring"] = &Item::__tostring;
		meta["index"] = sol::property(&Item::getIndex);
		meta["isActive"] = sol::property(&Item::getIsActive, &Item::setIsActive);
		meta["data"] = sol::property(&Item::getDataTable);
		meta["hasPhysics"] =
		    sol::property(&Item::getHasPhysics, &Item::setHasPhysics);
		meta["physicsSettled"] =
		    sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled);
		meta["isStatic"] = sol::property(&Item::getIsStatic, &Item::setIsStatic);
		meta["rigidBody"] = sol::property(&Item::getRigidBody);
		meta["grenadePrimer"] =
		    sol::property(&Item::getGrenadePrimer, &Item::setGrenadePrimer);
		meta["parentHuman"] = sol::property(&Item::getParentHuman);
		meta["parentItem"] = sol::property(&Item::getParentItem);

		meta["remove"] = &Item::remove;
		meta["mountItem"] = &Item::mountItem;
		meta["unmount"] = &Item::unmount;
		meta["speak"] = &Item::speak;
		meta["explode"] = &Item::explode;
		meta["setMemo"] = &Item::setMemo;
		meta["computerTransmitLine"] = &Item::computerTransmitLine;
		meta["computerIncrementLine"] = &Item::computerIncrementLine;
		meta["computerSetLine"] = &Item::computerSetLine;
		meta["computerSetColor"] = &Item::computerSetColor;
	}

	{
		auto meta = lua->new_usertype<Vehicle>("new", sol::no_constructor);
		meta["type"] = &Vehicle::type;
		meta["controllableState"] = &Vehicle::controllableState;
		meta["health"] = &Vehicle::health;
		meta["color"] = &Vehicle::color;
		meta["pos"] = &Vehicle::pos;
		meta["pos2"] = &Vehicle::pos2;
		meta["rot"] = &Vehicle::rot;
		meta["vel"] = &Vehicle::vel;
		// Messy but faster than using a table or some shit
		meta["windowState0"] = &Vehicle::windowState0;
		meta["windowState1"] = &Vehicle::windowState1;
		meta["windowState2"] = &Vehicle::windowState2;
		meta["windowState3"] = &Vehicle::windowState3;
		meta["windowState4"] = &Vehicle::windowState4;
		meta["windowState5"] = &Vehicle::windowState5;
		meta["windowState6"] = &Vehicle::windowState6;
		meta["windowState7"] = &Vehicle::windowState7;
		meta["gearX"] = &Vehicle::gearX;
		meta["steerControl"] = &Vehicle::steerControl;
		meta["gearY"] = &Vehicle::gearY;
		meta["gasControl"] = &Vehicle::gasControl;
		meta["bladeBodyID"] = &Vehicle::bladeBodyID;

		meta["class"] = sol::property(&Vehicle::getClass);
		meta["__tostring"] = &Vehicle::__tostring;
		meta["index"] = sol::property(&Vehicle::getIndex);
		meta["isActive"] =
		    sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive);
		meta["data"] = sol::property(&Vehicle::getDataTable);
		meta["lastDriver"] = sol::property(&Vehicle::getLastDriver);
		meta["rigidBody"] = sol::property(&Vehicle::getRigidBody);

		meta["updateType"] = &Vehicle::updateType;
		meta["updateDestruction"] = &Vehicle::updateDestruction;
		meta["remove"] = &Vehicle::remove;
	}

	{
		auto meta = lua->new_usertype<Bullet>("new", sol::no_constructor);
		meta["type"] = &Bullet::type;
		meta["time"] = &Bullet::time;
		meta["lastPos"] = &Bullet::lastPos;
		meta["pos"] = &Bullet::pos;
		meta["vel"] = &Bullet::vel;

		meta["class"] = sol::property(&Bullet::getClass);
		meta["player"] = sol::property(&Bullet::getPlayer);
	}

	{
		auto meta = lua->new_usertype<Bone>("new", sol::no_constructor);
		meta["pos"] = &Bone::pos;
		meta["pos2"] = &Bone::pos2;

		meta["class"] = sol::property(&Bone::getClass);
	}

	{
		auto meta = lua->new_usertype<RigidBody>("new", sol::no_constructor);
		meta["type"] = &RigidBody::type;
		meta["unk0"] = &RigidBody::unk0;
		meta["mass"] = &RigidBody::mass;
		meta["pos"] = &RigidBody::pos;
		meta["vel"] = &RigidBody::vel;
		meta["rot"] = &RigidBody::rot;
		meta["rot2"] = &RigidBody::rot2;

		meta["class"] = sol::property(&RigidBody::getClass);
		meta["__tostring"] = &RigidBody::__tostring;
		meta["index"] = sol::property(&RigidBody::getIndex);
		meta["isActive"] =
		    sol::property(&RigidBody::getIsActive, &RigidBody::setIsActive);
		meta["data"] = sol::property(&RigidBody::getDataTable);
		meta["isSettled"] =
		    sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled);

		meta["bondTo"] = &RigidBody::bondTo;
		meta["bondRotTo"] = &RigidBody::bondRotTo;
		meta["bondToLevel"] = &RigidBody::bondToLevel;
		meta["collideLevel"] = &RigidBody::collideLevel;
	}

	{
		auto meta = lua->new_usertype<Bond>("new", sol::no_constructor);
		meta["type"] = &Bond::type;
		meta["despawnTime"] = &Bond::despawnTime;
		meta["globalPos"] = &Bond::globalPos;
		meta["localPos"] = &Bond::localPos;
		meta["otherLocalPos"] = &Bond::otherLocalPos;

		meta["class"] = sol::property(&Bond::getClass);
		meta["__tostring"] = &Bond::__tostring;
		meta["index"] = sol::property(&Bond::getIndex);
		meta["isActive"] = sol::property(&Bond::getIsActive, &Bond::setIsActive);
		meta["body"] = sol::property(&Bond::getBody);
		meta["otherBody"] = sol::property(&Bond::getOtherBody);
	}

	{
		auto meta = lua->new_usertype<Action>("new", sol::no_constructor);
		meta["type"] = &Action::type;
		meta["a"] = &Action::a;
		meta["b"] = &Action::b;
		meta["c"] = &Action::c;
		meta["d"] = &Action::d;

		meta["class"] = sol::property(&Action::getClass);
	}

	{
		auto meta = lua->new_usertype<MenuButton>("new", sol::no_constructor);
		meta["id"] = &MenuButton::id;
		meta["text"] = sol::property(&MenuButton::getText, &MenuButton::setText);

		meta["class"] = sol::property(&MenuButton::getClass);
	}

	{
		auto meta = lua->new_usertype<Worker>(
		    "Worker", sol::constructors<ChildProcess(std::string)>());
		meta["stop"] = &Worker::stop;
		meta["sendMessage"] = &Worker::sendMessage;
		meta["receiveMessage"] = &Worker::receiveMessage;
	}

	{
		auto meta = lua->new_usertype<ChildProcess>(
		    "ChildProcess", sol::constructors<ChildProcess(std::string)>());
		meta["isRunning"] = &ChildProcess::isRunning;
		meta["terminate"] = &ChildProcess::terminate;
		meta["getExitCode"] = &ChildProcess::getExitCode;
		meta["receiveMessage"] = &ChildProcess::receiveMessage;
		meta["sendMessage"] = &ChildProcess::sendMessage;
		meta["setCPULimit"] = &ChildProcess::setCPULimit;
		meta["setMemoryLimit"] = &ChildProcess::setMemoryLimit;
		meta["setFileSizeLimit"] = &ChildProcess::setFileSizeLimit;
		meta["getPriority"] = &ChildProcess::getPriority;
		meta["setPriority"] = &ChildProcess::setPriority;
	}

	{
		auto meta = lua->new_usertype<StreetLane>("new", sol::no_constructor);
		meta["direction"] = &StreetLane::direction;
		meta["posA"] = &StreetLane::posA;
		meta["posB"] = &StreetLane::posB;

		meta["class"] = sol::property(&StreetLane::getClass);
	}

	{
		auto meta = lua->new_usertype<Street>("new", sol::no_constructor);
		meta["trafficCuboidA"] = &Street::trafficCuboidA;
		meta["trafficCuboidB"] = &Street::trafficCuboidB;
		meta["numTraffic"] = &Street::numTraffic;

		meta["class"] = sol::property(&Street::getClass);
		meta["__tostring"] = &Street::__tostring;
		meta["index"] = sol::property(&Street::getIndex);
		meta["name"] = sol::property(&Street::getName);
		meta["intersectionA"] = sol::property(&Street::getIntersectionA);
		meta["intersectionB"] = sol::property(&Street::getIntersectionB);
		meta["numLanes"] = sol::property(&Street::getNumLanes);

		meta["getLane"] = &Street::getLane;
	}

	{
		auto meta =
		    lua->new_usertype<StreetIntersection>("new", sol::no_constructor);
		meta["pos"] = &StreetIntersection::pos;
		meta["lightsState"] = &StreetIntersection::lightsState;
		meta["lightsTimer"] = &StreetIntersection::lightsTimer;
		meta["lightsTimerMax"] = &StreetIntersection::lightsTimerMax;
		meta["lightEast"] = &StreetIntersection::lightEast;
		meta["lightSouth"] = &StreetIntersection::lightSouth;
		meta["lightWest"] = &StreetIntersection::lightWest;
		meta["lightNorth"] = &StreetIntersection::lightNorth;

		meta["class"] = sol::property(&StreetIntersection::getClass);
		meta["__tostring"] = &StreetIntersection::__tostring;
		meta["index"] = sol::property(&StreetIntersection::getIndex);
		meta["streetEast"] = sol::property(&StreetIntersection::getStreetEast);
		meta["streetSouth"] = sol::property(&StreetIntersection::getStreetSouth);
		meta["streetWest"] = sol::property(&StreetIntersection::getStreetWest);
		meta["streetNorth"] = sol::property(&StreetIntersection::getStreetNorth);
	}

	(*lua)["flagStateForReset"] = Lua::flagStateForReset;

	(*lua)["hook"] = lua->create_table();
	(*lua)["hook"]["persistentMode"] = hookMode;

	{
		auto eventTable = lua->create_table();
		(*lua)["event"] = eventTable;
		eventTable["sound"] =
		    sol::overload(Lua::event::sound, Lua::event::soundSimple);
		eventTable["explosion"] = Lua::event::explosion;
		eventTable["bulletHit"] = Lua::event::bulletHit;
	}

	{
		auto physicsTable = lua->create_table();
		(*lua)["physics"] = physicsTable;
		physicsTable["lineIntersectLevel"] = Lua::physics::lineIntersectLevel;
		physicsTable["lineIntersectHuman"] = Lua::physics::lineIntersectHuman;
		physicsTable["lineIntersectVehicle"] = Lua::physics::lineIntersectVehicle;
		physicsTable["lineIntersectTriangle"] = Lua::physics::lineIntersectTriangle;
		physicsTable["garbageCollectBullets"] = Lua::physics::garbageCollectBullets;
	}

	{
		auto chatTable = lua->create_table();
		(*lua)["chat"] = chatTable;
		chatTable["announce"] = Lua::chat::announce;
		chatTable["tellAdmins"] = Lua::chat::tellAdmins;
		chatTable["addRaw"] = Lua::chat::addRaw;
	}

	{
		auto accountsTable = lua->create_table();
		(*lua)["accounts"] = accountsTable;
		accountsTable["save"] = Lua::accounts::save;
		accountsTable["getCount"] = Lua::accounts::getCount;
		accountsTable["getAll"] = Lua::accounts::getAll;
		accountsTable["getByPhone"] = Lua::accounts::getByPhone;

		sol::table _meta = lua->create_table();
		accountsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::accounts::getCount;
		_meta["__index"] = Lua::accounts::getByIndex;
	}

	{
		auto playersTable = lua->create_table();
		(*lua)["players"] = playersTable;
		playersTable["getCount"] = Lua::players::getCount;
		playersTable["getAll"] = Lua::players::getAll;
		playersTable["getByPhone"] = Lua::players::getByPhone;
		playersTable["getNonBots"] = Lua::players::getNonBots;
		playersTable["createBot"] = Lua::players::createBot;

		sol::table _meta = lua->create_table();
		playersTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::players::getCount;
		_meta["__index"] = Lua::players::getByIndex;
	}

	{
		auto humansTable = lua->create_table();
		(*lua)["humans"] = humansTable;
		humansTable["getCount"] = Lua::humans::getCount;
		humansTable["getAll"] = Lua::humans::getAll;
		humansTable["create"] = Lua::humans::create;

		sol::table _meta = lua->create_table();
		humansTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::humans::getCount;
		_meta["__index"] = Lua::humans::getByIndex;
	}

	{
		auto itemTypesTable = lua->create_table();
		(*lua)["itemTypes"] = itemTypesTable;
		itemTypesTable["getCount"] = Lua::itemTypes::getCount;
		itemTypesTable["getAll"] = Lua::itemTypes::getAll;

		sol::table _meta = lua->create_table();
		itemTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::itemTypes::getCount;
		_meta["__index"] = Lua::itemTypes::getByIndex;
	}

	{
		auto itemsTable = lua->create_table();
		(*lua)["items"] = itemsTable;
		itemsTable["getCount"] = Lua::items::getCount;
		itemsTable["getAll"] = Lua::items::getAll;
		itemsTable["create"] =
		    sol::overload(Lua::items::create, Lua::items::createVel);
		itemsTable["createRope"] = Lua::items::createRope;

		sol::table _meta = lua->create_table();
		itemsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::items::getCount;
		_meta["__index"] = Lua::items::getByIndex;
	}

	{
		auto vehiclesTable = lua->create_table();
		(*lua)["vehicles"] = vehiclesTable;
		vehiclesTable["getCount"] = Lua::vehicles::getCount;
		vehiclesTable["getAll"] = Lua::vehicles::getAll;
		vehiclesTable["create"] =
		    sol::overload(Lua::vehicles::create, Lua::vehicles::createVel);

		sol::table _meta = lua->create_table();
		vehiclesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::vehicles::getCount;
		_meta["__index"] = Lua::vehicles::getByIndex;
	}

	{
		auto bulletsTable = lua->create_table();
		(*lua)["bullets"] = bulletsTable;
		bulletsTable["getCount"] = Lua::bullets::getCount;
		bulletsTable["getAll"] = Lua::bullets::getAll;
	}

	{
		auto rigidBodiesTable = lua->create_table();
		(*lua)["rigidBodies"] = rigidBodiesTable;
		rigidBodiesTable["getCount"] = Lua::rigidBodies::getCount;
		rigidBodiesTable["getAll"] = Lua::rigidBodies::getAll;

		sol::table _meta = lua->create_table();
		rigidBodiesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::rigidBodies::getCount;
		_meta["__index"] = Lua::rigidBodies::getByIndex;
	}

	{
		auto bondsTable = lua->create_table();
		(*lua)["bonds"] = bondsTable;
		bondsTable["getCount"] = Lua::bonds::getCount;
		bondsTable["getAll"] = Lua::bonds::getAll;

		sol::table _meta = lua->create_table();
		bondsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::bonds::getCount;
		_meta["__index"] = Lua::bonds::getByIndex;
	}

	{
		auto streetsTable = lua->create_table();
		(*lua)["streets"] = streetsTable;
		streetsTable["getCount"] = Lua::streets::getCount;
		streetsTable["getAll"] = Lua::streets::getAll;

		sol::table _meta = lua->create_table();
		streetsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::streets::getCount;
		_meta["__index"] = Lua::streets::getByIndex;
	}

	{
		auto intersectionsTable = lua->create_table();
		(*lua)["intersections"] = intersectionsTable;
		intersectionsTable["getCount"] = Lua::intersections::getCount;
		intersectionsTable["getAll"] = Lua::intersections::getAll;

		sol::table _meta = lua->create_table();
		intersectionsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::intersections::getCount;
		_meta["__index"] = Lua::intersections::getByIndex;
	}

	{
		auto memoryTable = lua->create_table();
		(*lua)["memory"] = memoryTable;
		memoryTable["getBaseAddress"] = Lua::memory::getBaseAddress;
		memoryTable["getAddress"] = sol::overload(
		    &Lua::memory::getAddressOfConnection, &Lua::memory::getAddressOfAccount,
		    &Lua::memory::getAddressOfPlayer, &Lua::memory::getAddressOfHuman,
		    &Lua::memory::getAddressOfItemType, &Lua::memory::getAddressOfItem,
		    &Lua::memory::getAddressOfVehicle, &Lua::memory::getAddressOfBullet,
		    &Lua::memory::getAddressOfBone, &Lua::memory::getAddressOfRigidBody,
		    &Lua::memory::getAddressOfBond, &Lua::memory::getAddressOfAction,
		    &Lua::memory::getAddressOfMenuButton,
		    &Lua::memory::getAddressOfStreetLane, &Lua::memory::getAddressOfStreet,
		    &Lua::memory::getAddressOfStreetIntersection);
		memoryTable["readByte"] = Lua::memory::readByte;
		memoryTable["readUByte"] = Lua::memory::readUByte;
		memoryTable["readShort"] = Lua::memory::readShort;
		memoryTable["readUShort"] = Lua::memory::readUShort;
		memoryTable["readInt"] = Lua::memory::readInt;
		memoryTable["readUInt"] = Lua::memory::readUInt;
		memoryTable["readLong"] = Lua::memory::readLong;
		memoryTable["readULong"] = Lua::memory::readULong;
		memoryTable["readFloat"] = Lua::memory::readFloat;
		memoryTable["readDouble"] = Lua::memory::readDouble;
		memoryTable["readBytes"] = Lua::memory::readBytes;
		memoryTable["writeByte"] = Lua::memory::writeByte;
		memoryTable["writeUByte"] = Lua::memory::writeUByte;
		memoryTable["writeShort"] = Lua::memory::writeShort;
		memoryTable["writeUShort"] = Lua::memory::writeUShort;
		memoryTable["writeInt"] = Lua::memory::writeInt;
		memoryTable["writeUInt"] = Lua::memory::writeUInt;
		memoryTable["writeLong"] = Lua::memory::writeLong;
		memoryTable["writeULong"] = Lua::memory::writeULong;
		memoryTable["writeFloat"] = Lua::memory::writeFloat;
		memoryTable["writeDouble"] = Lua::memory::writeDouble;
		memoryTable["writeBytes"] = Lua::memory::writeBytes;
	}

	(*lua)["RESET_REASON_BOOT"] = RESET_REASON_BOOT;
	(*lua)["RESET_REASON_ENGINECALL"] = RESET_REASON_ENGINECALL;
	(*lua)["RESET_REASON_LUARESET"] = RESET_REASON_LUARESET;
	(*lua)["RESET_REASON_LUACALL"] = RESET_REASON_LUACALL;

	(*lua)["STATE_PREGAME"] = 1;
	(*lua)["STATE_GAME"] = 2;
	(*lua)["STATE_RESTARTING"] = 3;

	(*lua)["TYPE_DRIVING"] = 1;
	(*lua)["TYPE_RACE"] = 2;
	(*lua)["TYPE_ROUND"] = 3;
	(*lua)["TYPE_WORLD"] = 4;
	(*lua)["TYPE_TERMINATOR"] = 5;
	(*lua)["TYPE_COOP"] = 6;
	(*lua)["TYPE_VERSUS"] = 7;

	Console::log(LUA_PREFIX "Running " LUA_ENTRY_FILE "...\n");

	sol::load_result load = lua->load_file(LUA_ENTRY_FILE);
	if (noLuaCallError(&load)) {
		sol::protected_function_result res = load();
		if (noLuaCallError(&res)) {
			Console::log(LUA_PREFIX "No problems!\n");
		}
	}
}

static inline uintptr_t getBaseAddress() {
	std::ifstream file("/proc/self/maps");
	std::string line;
	// First line
	std::getline(file, line);
	auto pos = line.find("-");
	auto truncated = line.substr(0, pos);

	return std::stoul(truncated, nullptr, 16);
}

static inline void printBaseAddress(uintptr_t base) {
	std::ostringstream stream;

	stream << RS_PREFIX "Base address is ";
	stream << std::showbase << std::hex;
	stream << base;
	stream << "\n";

	Console::log(stream.str());
}

static inline void locateMemory(uintptr_t base) {
	Engine::version = (unsigned int*)(base + 0x2D5F08);
	Engine::subVersion = (unsigned int*)(base + 0x2D5F04);
	Engine::serverName = (char*)(base + 0x24EE4234);
	Engine::serverPort = (unsigned int*)(base + 0x1CC6CE80);
	Engine::numEvents = (unsigned int*)(base + 0x4532f244);
	Engine::isPassworded = (int*)(base + 0x24EE4644);
	Engine::password = (char*)(base + 0x1CC6D48C);
	Engine::maxPlayers = (int*)(base + 0x24EE4648);

	Engine::gameType = (int*)(base + 0x443F3988);
	Engine::mapName = (char*)(base + 0x443F398C);
	Engine::loadedMapName = (char*)(base + 0x3C2EEFE4);
	Engine::gameState = (int*)(base + 0x443F3BA4);
	Engine::gameTimer = (int*)(base + 0x443F3BAC);
	Engine::sunTime = (unsigned int*)(base + 0x9846CC0);
	Engine::isLevelLoaded = (int*)(base + 0x3C2EEFE0);
	Engine::gravity = (float*)(base + 0xC72AC);
	pryMemory(Engine::gravity, 1);
	Engine::originalGravity = *Engine::gravity;

	Engine::lineIntersectResult = (RayCastResult*)(base + 0x55E44E00);

	Engine::connections = (Connection*)(base + 0x43ACE0);
	Engine::accounts = (Account*)(base + 0x334F6D0);
	Engine::players = (Player*)(base + 0x19BC9CC0);
	Engine::humans = (Human*)(base + 0x8B1D4A8);
	Engine::vehicles = (Vehicle*)(base + 0x20DEF320);
	Engine::itemTypes = (ItemType*)(base + 0x5A088680);
	Engine::items = (Item*)(base + 0x7FE2160);
	Engine::bullets = (Bullet*)(base + 0x4355E260);
	Engine::bodies = (RigidBody*)(base + 0x2DACC0);
	Engine::bonds = (Bond*)(base + 0x24964220);
	Engine::streets = (Street*)(base + 0x3C311030);
	Engine::streetIntersections = (StreetIntersection*)(base + 0x3C2EF02C);

	Engine::numConnections = (unsigned int*)(base + 0x4532F468);
	Engine::numBullets = (unsigned int*)(base + 0x4532F240);
	Engine::numStreets = (unsigned int*)(base + 0x3C31102C);
	Engine::numStreetIntersections = (unsigned int*)(base + 0x3C2EF024);

	Engine::subRosaPuts = (Engine::subRosaPutsFunc)(base + 0x1CF0);
	Engine::subRosa__printf_chk =
	    (Engine::subRosa__printf_chkFunc)(base + 0x1FE0);

	Engine::resetGame = (Engine::voidFunc)(base + 0xB10B0);

	Engine::logicSimulation = (Engine::voidFunc)(base + 0xB7BF0);
	Engine::logicSimulationRace = (Engine::voidFunc)(base + 0xB3650);
	Engine::logicSimulationRound = (Engine::voidFunc)(base + 0xB3DD0);
	Engine::logicSimulationWorld = (Engine::voidFunc)(base + 0xB71A0);
	Engine::logicSimulationTerminator = (Engine::voidFunc)(base + 0xB4D50);
	Engine::logicSimulationCoop = (Engine::voidFunc)(base + 0xB3410);
	Engine::logicSimulationVersus = (Engine::voidFunc)(base + 0xB65F0);
	Engine::logicPlayerActions = (Engine::voidIndexFunc)(base + 0xA93A0);

	Engine::physicsSimulation = (Engine::voidFunc)(base + 0xA6CC0);
	Engine::serverReceive = (Engine::serverReceiveFunc)(base + 0xC0BB0);
	Engine::serverSend = (Engine::voidFunc)(base + 0xBDBA0);
	Engine::bulletSimulation = (Engine::voidFunc)(base + 0x98960);
	Engine::bulletTimeToLive = (Engine::voidFunc)(base + 0x181B0);

	Engine::saveAccountsServer = (Engine::voidFunc)(base + 0x6CC0);

	Engine::createAccountByJoinTicket =
	    (Engine::createAccountByJoinTicketFunc)(base + 0x65D0);
	Engine::serverSendConnectResponse =
	    (Engine::serverSendConnectResponseFunc)(base + 0xB8FD0);

	Engine::scenarioArmHuman = (Engine::scenarioArmHumanFunc)(base + 0x4FDD0);
	Engine::linkItem = (Engine::linkItemFunc)(base + 0x2B060);
	Engine::itemSetMemo = (Engine::itemSetMemoFunc)(base + 0x25F80);
	Engine::itemComputerTransmitLine =
	    (Engine::itemComputerTransmitLineFunc)(base + 0x26100);
	Engine::itemComputerIncrementLine = (Engine::voidIndexFunc)(base + 0x263a0);
	Engine::itemComputerInput = (Engine::itemComputerInputFunc)(base + 0x4e620);

	Engine::humanApplyDamage = (Engine::humanApplyDamageFunc)(base + 0x1E1D0);
	Engine::humanCollisionVehicle =
	    (Engine::humanCollisionVehicleFunc)(base + 0x7AF50);
	Engine::humanGrabbing = (Engine::voidIndexFunc)(base + 0xA16D0);
	Engine::grenadeExplosion = (Engine::voidIndexFunc)(base + 0x2A990);
	Engine::serverPlayerMessage =
	    (Engine::serverPlayerMessageFunc)(base + 0xA7B80);
	Engine::playerAI = (Engine::voidIndexFunc)(base + 0x96F80);
	Engine::playerDeathTax = (Engine::voidIndexFunc)(base + 0x2D70);
	Engine::createBondRigidBodyToRigidBody =
	    (Engine::createBondRigidBodyToRigidBodyFunc)(base + 0x12CC0);
	Engine::createBondRigidBodyRotRigidBody =
	    (Engine::createBondRigidBodyRotRigidBodyFunc)(base + 0x12f70);
	Engine::createBondRigidBodyToLevel =
	    (Engine::createBondRigidBodyToLevelFunc)(base + 0x12B80);
	Engine::addCollisionRigidBodyOnRigidBody =
	    (Engine::addCollisionRigidBodyOnRigidBodyFunc)(base + 0x13070);
	Engine::addCollisionRigidBodyOnLevel =
	    (Engine::addCollisionRigidBodyOnLevelFunc)(base + 0x13220);

	Engine::createPlayer = (Engine::createPlayerFunc)(base + 0x40EE0);
	Engine::deletePlayer = (Engine::voidIndexFunc)(base + 0x411D0);
	Engine::createHuman = (Engine::createHumanFunc)(base + 0x66D10);
	Engine::deleteHuman = (Engine::voidIndexFunc)(base + 0x3EB0);
	Engine::createItem = (Engine::createItemFunc)(base + 0x4DDE0);
	Engine::deleteItem = (Engine::voidIndexFunc)(base + 0x2C180);
	Engine::createRope = (Engine::createRopeFunc)(base + 0x4F150);
	Engine::createVehicle = (Engine::createVehicleFunc)(base + 0x4CEA0);
	Engine::deleteVehicle = (Engine::voidIndexFunc)(base + 0x42A0);
	Engine::createRigidBody = (Engine::createRigidBodyFunc)(base + 0x4cc90);

	Engine::createEventMessage = (Engine::createEventMessageFunc)(base + 0x29C0);
	Engine::createEventUpdatePlayer = (Engine::voidIndexFunc)(base + 0x2BE0);
	Engine::createEventUpdatePlayerFinance =
	    (Engine::voidIndexFunc)(base + 0x2D00);
	Engine::createEventCreateVehicle = (Engine::voidIndexFunc)(base + 0x2AE0);
	Engine::createEventUpdateVehicle =
	    (Engine::createEventUpdateVehicleFunc)(base + 0x41C0);
	Engine::createEventSound = (Engine::createEventSoundFunc)(base + 0x3CC0);
	Engine::createEventExplosion =
	    (Engine::createEventExplosionFunc)(base + 0x45A0);
	Engine::createEventBulletHit =
	    (Engine::createEventBulletHitFunc)(base + 0x4110);

	Engine::lineIntersectHuman = (Engine::lineIntersectHumanFunc)(base + 0x23AB0);
	Engine::lineIntersectLevel = (Engine::lineIntersectLevelFunc)(base + 0x7C470);
	Engine::lineIntersectVehicle =
	    (Engine::lineIntersectVehicleFunc)(base + 0x95590);
	Engine::lineIntersectTriangle =
	    (Engine::lineIntersectTriangleFunc)(base + 0x6aa70);
}

static inline void installHook(
    const char* name, subhook::Hook& hook, void* source, void* destination,
    subhook::HookFlags flags = subhook::HookFlags::HookFlag64BitOffset) {
	if (!hook.Install(source, destination, flags)) {
		std::ostringstream stream;
		stream << RS_PREFIX "Hook " << name << " failed to install";

		throw std::runtime_error(stream.str());
	}
}

#define INSTALL(name)                                               \
	installHook(#name "Hook", Hooks::name##Hook, (void*)Engine::name, \
	            (void*)Hooks::name);

static inline void installHooks() {
	INSTALL(subRosaPuts);
	INSTALL(subRosaPuts);
	INSTALL(subRosa__printf_chk);
	INSTALL(resetGame);
	INSTALL(logicSimulation);
	INSTALL(logicSimulationRace);
	INSTALL(logicSimulationRound);
	INSTALL(logicSimulationWorld);
	INSTALL(logicSimulationTerminator);
	INSTALL(logicSimulationCoop);
	INSTALL(logicSimulationVersus);
	INSTALL(logicPlayerActions);
	INSTALL(physicsSimulation);
	INSTALL(serverReceive);
	INSTALL(serverSend);
	INSTALL(bulletSimulation);
	INSTALL(saveAccountsServer);
	INSTALL(createAccountByJoinTicket);
	INSTALL(serverSendConnectResponse);
	INSTALL(linkItem);
	INSTALL(itemComputerInput);
	INSTALL(humanApplyDamage);
	INSTALL(humanCollisionVehicle);
	INSTALL(humanGrabbing);
	INSTALL(grenadeExplosion);
	INSTALL(serverPlayerMessage);
	INSTALL(playerAI);
	INSTALL(playerDeathTax);
	INSTALL(addCollisionRigidBodyOnRigidBody);
	INSTALL(createPlayer);
	INSTALL(deletePlayer);
	INSTALL(createHuman);
	INSTALL(deleteHuman);
	INSTALL(createItem);
	INSTALL(deleteItem);
	INSTALL(createVehicle);
	INSTALL(deleteVehicle);
	INSTALL(createRigidBody);
	INSTALL(createEventMessage);
	INSTALL(createEventUpdatePlayer);
	INSTALL(createEventUpdateVehicle);
	INSTALL(createEventBulletHit);
	INSTALL(lineIntersectHuman);
}

static inline void attachSignalHandler() {
	struct sigaction action;
	action.sa_handler = Console::handleInterruptSignal;

	if (sigaction(SIGINT, &action, nullptr) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}

static void attach() {
	// Don't load self into future child processes
	unsetenv("LD_PRELOAD");

	attachSignalHandler();

	Console::log(RS_PREFIX "Assuming 37c\n");

	Console::log(RS_PREFIX "Locating memory...\n");
	Lua::memory::baseAddress = getBaseAddress();
	printBaseAddress(Lua::memory::baseAddress);
	locateMemory(Lua::memory::baseAddress);

	Console::log(RS_PREFIX "Installing hooks...\n");
	installHooks();

	Console::log(RS_PREFIX "Waiting for engine init...\n");
}

int __attribute__((constructor)) Entry() {
	std::thread mainThread(attach);
	mainThread.detach();
	return 0;
}

int __attribute__((destructor)) Destroy() {
	if (lua != nullptr) {
		delete lua;
		lua = nullptr;
	}
	return 0;
}