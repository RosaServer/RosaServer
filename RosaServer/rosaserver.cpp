#include "rosaserver.h"
#include <sys/mman.h>
#include <cerrno>

#define handle_error(msg)               \
	do                                    \
	{                                     \
		std::cout << __LINE__ << std::endl; \
		perror(msg);                        \
		exit(EXIT_FAILURE);                 \
	} while (0)

static unsigned int* version;
static unsigned int* subVersion;
static char* serverName;
static unsigned int* serverPort;
static unsigned int* numEvents;

static int* isPassworded;
static char* password;
static int* maxPlayers;

static int* isLevelLoaded;
static float* gravity;
static float originalGravity;

static void pryMemory(void* address, size_t numPages)
{
	size_t pageSize = sysconf(_SC_PAGE_SIZE);

	uintptr_t page = (uintptr_t)address;
	page -= (page % pageSize);

	if (mprotect((void*)page, pageSize * numPages, PROT_WRITE | PROT_READ) == 0)
	{
		std::ostringstream stream;

		stream << RS_PREFIX "Successfully pried open page at ";
		stream << std::showbase << std::hex;
		stream << static_cast<uintptr_t>(page);
		stream << "\n";

		Console::log(stream.str());
	}
	else
	{
		handle_error("mprotect");
	}
}

/*static subhook::Hook _test_hook;
typedef int(*_test_func)(int, Vector*, RotMatrix*, Vector*, Vector*, float);
static _test_func _test;

int h__test(int type, Vector* pos, RotMatrix* rot, Vector* vel, Vector* scale, float mass) {
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("Test", type, pos, rot, vel, scale, mass);
		noLuaCallError(&res);
	}

	subhook::ScopedHookRemove remove(&_test_hook);
	return _test(type, pos, rot, vel, scale, mass);
}*/

struct Server
{
	const int TPS = 60;

	const char* getClass() const
	{
		return "Server";
	}
	int getPort() const
	{
		return *serverPort;
	}
	char* getName() const
	{
		return serverName;
	}
	void setName(const char* newName) const
	{
		strncpy(serverName, newName, 31);
	}
	char* getPassword() const
	{
		return password;
	}
	void setPassword(const char* newPassword) const
	{
		strncpy(password, newPassword, 31);
		*isPassworded = newPassword[0] != 0;
	}
	int getMaxPlayers() const
	{
		return *maxPlayers;
	}
	void setMaxPlayers(int max) const
	{
		*maxPlayers = max;
	}
	int getType() const
	{
		return *gameType;
	}
	void setType(int type) const
	{
		*gameType = type;
	}
	char* getLevelName() const
	{
		return mapName;
	}
	void setLevelName(const char* newName) const
	{
		strncpy(mapName, newName, 31);
	}
	char* getLoadedLevelName() const
	{
		return loadedMapName;
	}
	bool getIsLevelLoaded() const
	{
		return *isLevelLoaded;
	}
	void setIsLevelLoaded(bool b) const
	{
		*isLevelLoaded = b;
	}
	float getGravity() const
	{
		return *gravity;
	}
	void setGravity(float g) const
	{
		*gravity = g;
	}
	float getDefaultGravity() const
	{
		return originalGravity;
	}
	int getState() const
	{
		return *gameState;
	}
	void setState(int state) const
	{
		*gameState = state;
	}
	int getTime() const
	{
		return *gameTimer;
	}
	void setTime(int time) const
	{
		*gameTimer = time;
	}
	int getSunTime() const
	{
		return *sunTime;
	}
	void setSunTime(int time) const
	{
		*sunTime = time % 5184000;
	}
	std::string getVersion() const
	{
		std::ostringstream stream;
		stream << *version << (char)(*subVersion + 97);
		return stream.str();
	}
	unsigned int getVersionMajor() const
	{
		return *version;
	}
	unsigned int getVersionMinor() const
	{
		return *subVersion;
	}
	unsigned int getNumEvents() const
	{
		return *numEvents;
	}

	void setConsoleTitle(const char* title) const
	{
		printf("\033]0;%s\007", title);
	}
	void reset() const
	{
		hookAndReset(RESET_REASON_LUACALL);
	}
};
static Server* server;

void defineThreadSafeAPIs(sol::state* state)
{
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

	(*state)["print"] = l_print;

	(*state)["Vector"] = sol::overload(l_Vector, l_Vector_3f);
	(*state)["RotMatrix"] = l_RotMatrix;

	(*state)["os"]["listDirectory"] = l_os_listDirectory;
	(*state)["os"]["createDirectory"] = l_os_createDirectory;

	(*state)["os"]["realClock"] = l_os_realClock;

	{
		auto httpTable = state->create_table();
		(*state)["http"] = httpTable;
		httpTable["getSync"] = l_http_getSync;
		httpTable["postSync"] = l_http_postSync;
	}
}

void luaInit(bool redo)
{
	std::lock_guard<std::mutex> guard(stateResetMutex);
	requestQueue = std::queue<LuaHTTPRequest>();
	responseQueue = std::queue<LuaHTTPResponse>();

	if (redo)
	{
		Console::log(LUA_PREFIX "Resetting state...\n");
		delete server;

		for (int i = 0; i < MAXNUMOFPLAYERS; i++)
		{
			if (playerDataTables[i])
			{
				delete playerDataTables[i];
				playerDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < MAXNUMOFHUMANS; i++)
		{
			if (humanDataTables[i])
			{
				delete humanDataTables[i];
				humanDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < MAXNUMOFITEMS; i++)
		{
			if (itemDataTables[i])
			{
				delete itemDataTables[i];
				itemDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < MAXNUMOFVEHICLES; i++)
		{
			if (vehicleDataTables[i])
			{
				delete vehicleDataTables[i];
				vehicleDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < MAXNUMOFRIGIDBODIES; i++)
		{
			if (bodyDataTables[i])
			{
				delete bodyDataTables[i];
				bodyDataTables[i] = nullptr;
			}
		}

		delete lua;
	}
	else
	{
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
		meta["password"] = sol::property(&Server::getPassword, &Server::setPassword);
		meta["maxPlayers"] = sol::property(&Server::getMaxPlayers, &Server::setMaxPlayers);
		meta["type"] = sol::property(&Server::getType, &Server::setType);
		meta["levelToLoad"] = sol::property(&Server::getLevelName, &Server::setLevelName);
		meta["loadedLevel"] = sol::property(&Server::getLoadedLevelName);
		meta["isLevelLoaded"] = sol::property(&Server::getIsLevelLoaded, &Server::setIsLevelLoaded);
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
		meta["adminVisible"] = sol::property(&Connection::getAdminVisible, &Connection::setAdminVisible);
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
		meta["isActive"] = sol::property(&Player::getIsActive, &Player::setIsActive);
		meta["data"] = sol::property(&Player::getDataTable);
		meta["name"] = sol::property(&Player::getName, &Player::setName);
		meta["isAdmin"] = sol::property(&Player::getIsAdmin, &Player::setIsAdmin);
		meta["isReady"] = sol::property(&Player::getIsReady, &Player::setIsReady);
		meta["isBot"] = sol::property(&Player::getIsBot, &Player::setIsBot);
		meta["human"] = sol::property(&Player::getHuman, &Player::setHuman);
		meta["connection"] = sol::property(&Player::getConnection);
		meta["account"] = sol::property(&Player::getAccount, &Player::setAccount);
		meta["botDestination"] = sol::property(&Player::getBotDestination, &Player::setBotDestination);

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
		meta["isImmortal"] = sol::property(&Human::getIsImmortal, &Human::setIsImmortal);
		meta["isOnGround"] = sol::property(&Human::getIsOnGround);
		meta["isStanding"] = sol::property(&Human::getIsStanding);
		meta["isBleeding"] = sol::property(&Human::getIsBleeding, &Human::setIsBleeding);
		meta["player"] = sol::property(&Human::getPlayer, &Human::setPlayer);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);
		meta["rightHandItem"] = sol::property(&Human::getRightHandItem);
		meta["leftHandItem"] = sol::property(&Human::getLeftHandItem);
		meta["rightHandGrab"] = sol::property(&Human::getRightHandGrab, &Human::setRightHandGrab);
		meta["leftHandGrab"] = sol::property(&Human::getLeftHandGrab, &Human::setLeftHandGrab);
		meta["isAppearanceDirty"] = sol::property(&Human::getIsAppearanceDirty, &Human::setIsAppearanceDirty);

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
		meta["hasPhysics"] = sol::property(&Item::getHasPhysics, &Item::setHasPhysics);
		meta["physicsSettled"] = sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled);
		meta["isStatic"] = sol::property(&Item::getIsStatic, &Item::setIsStatic);
		meta["rigidBody"] = sol::property(&Item::getRigidBody);
		meta["grenadePrimer"] = sol::property(&Item::getGrenadePrimer, &Item::setGrenadePrimer);
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
		meta["isActive"] = sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive);
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
		meta["isActive"] = sol::property(&RigidBody::getIsActive, &RigidBody::setIsActive);
		meta["data"] = sol::property(&RigidBody::getDataTable);
		meta["isSettled"] = sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled);

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
		auto meta = lua->new_usertype<Worker>("Worker", sol::constructors<ChildProcess(std::string)>());
		meta["stop"] = &Worker::stop;
		meta["sendMessage"] = &Worker::sendMessage;
		meta["receiveMessage"] = &Worker::receiveMessage;
	}

	{
		auto meta = lua->new_usertype<ChildProcess>("ChildProcess", sol::constructors<ChildProcess(std::string)>());
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
		auto meta = lua->new_usertype<StreetIntersection>("new", sol::no_constructor);
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

	(*lua)["flagStateForReset"] = l_flagStateForReset;

	(*lua)["hook"] = lua->create_table();
	(*lua)["hook"]["persistentMode"] = hookMode;

	(*lua)["http"]["get"] = l_http_get;
	(*lua)["http"]["post"] = l_http_post;

	{
		auto eventTable = lua->create_table();
		(*lua)["event"] = eventTable;
		eventTable["sound"] = sol::overload(l_event_sound, l_event_soundSimple);
		eventTable["explosion"] = l_event_explosion;
		eventTable["bulletHit"] = l_event_bulletHit;
	}

	{
		auto physicsTable = lua->create_table();
		(*lua)["physics"] = physicsTable;
		physicsTable["lineIntersectLevel"] = l_physics_lineIntersectLevel;
		physicsTable["lineIntersectHuman"] = l_physics_lineIntersectHuman;
		physicsTable["lineIntersectVehicle"] = l_physics_lineIntersectVehicle;
		physicsTable["lineIntersectTriangle"] = l_physics_lineIntersectTriangle;
		physicsTable["garbageCollectBullets"] = l_physics_garbageCollectBullets;
	}

	{
		auto chatTable = lua->create_table();
		(*lua)["chat"] = chatTable;
		chatTable["announce"] = l_chat_announce;
		chatTable["tellAdmins"] = l_chat_tellAdmins;
		chatTable["addRaw"] = l_chat_addRaw;
	}

	{
		auto accountsTable = lua->create_table();
		(*lua)["accounts"] = accountsTable;
		accountsTable["save"] = l_accounts_save;
		accountsTable["getCount"] = l_accounts_getCount;
		accountsTable["getAll"] = l_accounts_getAll;
		accountsTable["getByPhone"] = l_accounts_getByPhone;

		sol::table _meta = lua->create_table();
		accountsTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_accounts_getCount;
		_meta["__index"] = l_accounts_getByIndex;
	}

	{
		auto playersTable = lua->create_table();
		(*lua)["players"] = playersTable;
		playersTable["getCount"] = l_players_getCount;
		playersTable["getAll"] = l_players_getAll;
		playersTable["getByPhone"] = l_players_getByPhone;
		playersTable["getNonBots"] = l_players_getNonBots;
		playersTable["createBot"] = l_players_createBot;

		sol::table _meta = lua->create_table();
		playersTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_players_getCount;
		_meta["__index"] = l_players_getByIndex;
	}

	{
		auto humansTable = lua->create_table();
		(*lua)["humans"] = humansTable;
		humansTable["getCount"] = l_humans_getCount;
		humansTable["getAll"] = l_humans_getAll;
		humansTable["create"] = l_humans_create;

		sol::table _meta = lua->create_table();
		humansTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_humans_getCount;
		_meta["__index"] = l_humans_getByIndex;
	}

	{
		auto itemTypesTable = lua->create_table();
		(*lua)["itemTypes"] = itemTypesTable;
		itemTypesTable["getCount"] = l_itemTypes_getCount;
		itemTypesTable["getAll"] = l_itemTypes_getAll;

		sol::table _meta = lua->create_table();
		itemTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_itemTypes_getCount;
		_meta["__index"] = l_itemTypes_getByIndex;
	}

	{
		auto itemsTable = lua->create_table();
		(*lua)["items"] = itemsTable;
		itemsTable["getCount"] = l_items_getCount;
		itemsTable["getAll"] = l_items_getAll;
		itemsTable["create"] = sol::overload(l_items_create, l_items_createVel);
		itemsTable["createRope"] = l_items_createRope;

		sol::table _meta = lua->create_table();
		itemsTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_items_getCount;
		_meta["__index"] = l_items_getByIndex;
	}

	{
		auto vehiclesTable = lua->create_table();
		(*lua)["vehicles"] = vehiclesTable;
		vehiclesTable["getCount"] = l_vehicles_getCount;
		vehiclesTable["getAll"] = l_vehicles_getAll;
		vehiclesTable["create"] = sol::overload(l_vehicles_create, l_vehicles_createVel);
		
		sol::table _meta = lua->create_table();
		vehiclesTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_vehicles_getCount;
		_meta["__index"] = l_vehicles_getByIndex;
	}

	{
		auto bulletsTable = lua->create_table();
		(*lua)["bullets"] = bulletsTable;
		bulletsTable["getCount"] = l_bullets_getCount;
		bulletsTable["getAll"] = l_bullets_getAll;
	}

	{
		auto rigidBodiesTable = lua->create_table();
		(*lua)["rigidBodies"] = rigidBodiesTable;
		rigidBodiesTable["getCount"] = l_rigidBodies_getCount;
		rigidBodiesTable["getAll"] = l_rigidBodies_getAll;
		
		sol::table _meta = lua->create_table();
		rigidBodiesTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_rigidBodies_getCount;
		_meta["__index"] = l_rigidBodies_getByIndex;
	}

	{
		auto bondsTable = lua->create_table();
		(*lua)["bonds"] = bondsTable;
		bondsTable["getCount"] = l_bonds_getCount;
		bondsTable["getAll"] = l_bonds_getAll;

		sol::table _meta = lua->create_table();
		bondsTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_bonds_getCount;
		_meta["__index"] = l_bonds_getByIndex;
	}

	{
		auto streetsTable = lua->create_table();
		(*lua)["streets"] = streetsTable;
		streetsTable["getCount"] = l_streets_getCount;
		streetsTable["getAll"] = l_streets_getAll;

		sol::table _meta = lua->create_table();
		streetsTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_streets_getCount;
		_meta["__index"] = l_streets_getByIndex;
	}

	{
		auto intersectionsTable = lua->create_table();
		(*lua)["intersections"] = intersectionsTable;
		intersectionsTable["getCount"] = l_intersections_getCount;
		intersectionsTable["getAll"] = l_intersections_getAll;

		sol::table _meta = lua->create_table();
		intersectionsTable[sol::metatable_key] = _meta;
		_meta["__len"] = l_intersections_getCount;
		_meta["__index"] = l_intersections_getByIndex;
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
	if (noLuaCallError(&load))
	{
		sol::protected_function_result res = load();
		if (noLuaCallError(&res))
		{
			Console::log(LUA_PREFIX "No problems!\n");
		}
	}
}

static inline unsigned long getBaseAddress()
{
	std::ifstream file("/proc/self/maps");
	std::string line;
	// First line
	std::getline(file, line);
	auto pos = line.find("-");
	auto truncated = line.substr(0, pos);

	return std::stoul(truncated, nullptr, 16);
}

static inline void printBaseAddress(unsigned long base)
{
	std::ostringstream stream;

	stream << RS_PREFIX "Base address is ";
	stream << std::showbase << std::hex;
	stream << base;
	stream << "\n";

	Console::log(stream.str());
}

static inline void locateMemory(unsigned long base)
{
	version = (unsigned int*)(base + 0x2D5F08);
	subVersion = (unsigned int*)(base + 0x2D5F04);
	serverName = (char*)(base + 0x24EE4234);
	serverPort = (unsigned int*)(base + 0x1CC6CE80);
	numEvents = (unsigned int*)(base + 0x4532f244);
	isPassworded = (int*)(base + 0x24EE4644);
	password = (char*)(base + 0x1CC6D48C);
	maxPlayers = (int*)(base + 0x24EE4648);

	gameType = (int*)(base + 0x443F3988);
	mapName = (char*)(base + 0x443F398C);
	loadedMapName = (char*)(base + 0x3C2EEFE4);
	gameState = (int*)(base + 0x443F3BA4);
	gameTimer = (int*)(base + 0x443F3BAC);
	sunTime = (unsigned int*)(base + 0x9846CC0);
	isLevelLoaded = (int*)(base + 0x3C2EEFE0);
	gravity = (float*)(base + 0xC72AC);
	pryMemory(gravity, 1);
	originalGravity = *gravity;

	lineIntersectResult = (RayCastResult*)(base + 0x55E44E00);

	connections = (Connection*)(base + 0x43ACE0);
	accounts = (Account*)(base + 0x334F6D0);
	players = (Player*)(base + 0x19BC9CC0);
	humans = (Human*)(base + 0x8B1D4A8);
	vehicles = (Vehicle*)(base + 0x20DEF320);
	itemTypes = (ItemType*)(base + 0x5A088680);
	items = (Item*)(base + 0x7FE2160);
	bullets = (Bullet*)(base + 0x4355E260);
	bodies = (RigidBody*)(base + 0x2DACC0);
	bonds = (Bond*)(base + 0x24964220);
	streets = (Street*)(base + 0x3C311030);
	streetIntersections = (StreetIntersection*)(base + 0x3C2EF02C);

	numConnections = (unsigned int*)(base + 0x4532F468);
	numBullets = (unsigned int*)(base + 0x4532F240);
	numStreets = (unsigned int*)(base + 0x3C31102C);
	numStreetIntersections = (unsigned int*)(base + 0x3C2EF024);

	//_test = (_test_func)(base + 0x4cc90);
	//pryMemory(&_test, 2);

	subrosa_puts = (subrosa_puts_func)(base + 0x1CF0);
	subrosa___printf_chk = (subrosa___printf_chk_func)(base + 0x1FE0);

	resetgame = (void_func)(base + 0xB10B0);

	logicsimulation = (void_func)(base + 0xB7BF0);
	logicsimulation_race = (void_func)(base + 0xB3650);
	logicsimulation_round = (void_func)(base + 0xB3DD0);
	logicsimulation_world = (void_func)(base + 0xB71A0);
	logicsimulation_terminator = (void_func)(base + 0xB4D50);
	logicsimulation_coop = (void_func)(base + 0xB3410);
	logicsimulation_versus = (void_func)(base + 0xB65F0);
	logic_playeractions = (void_index_func)(base + 0xA93A0);

	physicssimulation = (void_func)(base + 0xA6CC0);
	serverrecv = (serverrecv_func)(base + 0xC0BB0);
	serversend = (void_func)(base + 0xBDBA0);
	bulletsimulation = (void_func)(base + 0x98960);
	bullettimetolive = (void_func)(base + 0x181B0);

	saveaccountsserver = (void_func)(base + 0x6CC0);

	createaccount_jointicket = (createaccount_jointicket_func)(base + 0x65D0);
	server_sendconnectreponse = (server_sendconnectreponse_func)(base + 0xB8FD0);

	scenario_armhuman = (scenario_armhuman_func)(base + 0x4FDD0);
	linkitem = (linkitem_func)(base + 0x2B060);
	item_setmemo = (item_setmemo_func)(base + 0x25F80);
	item_computertransmitline = (item_computertransmitline_func)(base + 0x26100);
	item_computerincrementline = (void_index_func)(base + 0x263a0);
	item_computerinput = (item_computerinput_func)(base + 0x4e620);

	human_applydamage = (human_applydamage_func)(base + 0x1E1D0);
	human_collisionvehicle = (human_collisionvehicle_func)(base + 0x7AF50);
	human_grabbing = (void_index_func)(base + 0xA16D0);
	grenadeexplosion = (void_index_func)(base + 0x2A990);
	server_playermessage = (server_playermessage_func)(base + 0xA7B80);
	playerai = (void_index_func)(base + 0x96F80);
	playerdeathtax = (void_index_func)(base + 0x2D70);
	createbond_rigidbody_rigidbody = (createbond_rigidbody_rigidbody_func)(base + 0x12CC0);
	createbond_rigidbody_rot_rigidbody = (createbond_rigidbody_rot_rigidbody_func)(base + 0x12f70);
	createbond_rigidbody_level = (createbond_rigidbody_level_func)(base + 0x12B80);
	addcollision_rigidbody_rigidbody = (addcollision_rigidbody_rigidbody_func)(base + 0x13070);
	addcollision_rigidbody_level = (addcollision_rigidbody_level_func)(base + 0x13220);

	createplayer = (createplayer_func)(base + 0x40EE0);
	deleteplayer = (void_index_func)(base + 0x411D0);
	createhuman = (createhuman_func)(base + 0x66D10);
	deletehuman = (void_index_func)(base + 0x3EB0);
	createitem = (createitem_func)(base + 0x4DDE0);
	deleteitem = (void_index_func)(base + 0x2C180);
	createrope = (createrope_func)(base + 0x4F150);
	createobject = (createobject_func)(base + 0x4CEA0);
	deleteobject = (void_index_func)(base + 0x42A0);
	createrigidbody = (createrigidbody_func)(base + 0x4cc90);

	createevent_message = (createevent_message_func)(base + 0x29C0);
	createevent_updateplayer = (void_index_func)(base + 0x2BE0);
	createevent_updateplayer_finance = (void_index_func)(base + 0x2D00);
	createevent_createobject = (void_index_func)(base + 0x2AE0);
	createevent_updateobject = (createevent_updateobject_func)(base + 0x41C0);
	createevent_sound = (createevent_sound_func)(base + 0x3CC0);
	createevent_explosion = (createevent_explosion_func)(base + 0x45A0);
	createevent_bullethit = (createevent_bullethit_func)(base + 0x4110);

	lineintersecthuman = (lineintersecthuman_func)(base + 0x23AB0);
	lineintersectlevel = (lineintersectlevel_func)(base + 0x7C470);
	lineintersectobject = (lineintersectobject_func)(base + 0x95590);
	lineintersecttriangle = (lineintersecttriangle_func)(base + 0x6aa70);
}

static inline void installHook(
	const char* name, subhook::Hook& hook,
	void* source, void* destination,
	subhook::HookFlags flags = subhook::HookFlags::HookFlag64BitOffset
	)
{
	if (!hook.Install(source, destination, flags))
	{
		std::ostringstream stream;
		stream << RS_PREFIX "Hook " << name << " failed to install";

		throw std::runtime_error(stream.str());
	}
}

static inline void installHooks()
{
	//_test_hook.Install((void*)_test, (void*)h__test, HOOK_FLAGS);
	installHook("subrosa_puts_hook", subrosa_puts_hook, (void*)subrosa_puts, (void*)h_subrosa_puts);
	installHook("subrosa___printf_chk_hook", subrosa___printf_chk_hook, (void*)subrosa___printf_chk, (void*)h_subrosa___printf_chk);

	installHook("resetgame_hook", resetgame_hook, (void*)resetgame, (void*)h_resetgame);

	installHook("logicsimulation_hook", logicsimulation_hook, (void*)logicsimulation, (void*)h_logicsimulation);
	installHook("logicsimulation_race_hook", logicsimulation_race_hook, (void*)logicsimulation_race, (void*)h_logicsimulation_race);
	installHook("logicsimulation_round_hook", logicsimulation_round_hook, (void*)logicsimulation_round, (void*)h_logicsimulation_round);
	installHook("logicsimulation_world_hook", logicsimulation_world_hook, (void*)logicsimulation_world, (void*)h_logicsimulation_world);
	installHook("logicsimulation_terminator_hook", logicsimulation_terminator_hook, (void*)logicsimulation_terminator, (void*)h_logicsimulation_terminator);
	installHook("logicsimulation_coop_hook", logicsimulation_coop_hook, (void*)logicsimulation_coop, (void*)h_logicsimulation_coop);
	installHook("logicsimulation_versus_hook", logicsimulation_versus_hook, (void*)logicsimulation_versus, (void*)h_logicsimulation_versus);
	installHook("logic_playeractions_hook", logic_playeractions_hook, (void*)logic_playeractions, (void*)h_logic_playeractions);

	installHook("physicssimulation_hook", physicssimulation_hook, (void*)physicssimulation, (void*)h_physicssimulation);
	installHook("serverrecv_hook", serverrecv_hook, (void*)serverrecv, (void*)h_serverrecv);
	installHook("serversend_hook", serversend_hook, (void*)serversend, (void*)h_serversend);
	installHook("bulletsimulation_hook", bulletsimulation_hook, (void*)bulletsimulation, (void*)h_bulletsimulation);

	installHook("saveaccountsserver_hook", saveaccountsserver_hook, (void*)saveaccountsserver, (void*)h_saveaccountsserver);

	installHook("createaccount_jointicket_hook", createaccount_jointicket_hook, (void*)createaccount_jointicket, (void*)h_createaccount_jointicket);
	installHook("server_sendconnectreponse_hook", server_sendconnectreponse_hook, (void*)server_sendconnectreponse, (void*)h_server_sendconnectreponse);

	installHook("linkitem_hook", linkitem_hook, (void*)linkitem, (void*)h_linkitem);
	installHook("item_computerinput_hook", item_computerinput_hook, (void*)item_computerinput, (void*)h_item_computerinput);
	installHook("human_applydamage_hook", human_applydamage_hook, (void*)human_applydamage, (void*)h_human_applydamage);
	installHook("human_collisionvehicle_hook", human_collisionvehicle_hook, (void*)human_collisionvehicle, (void*)h_human_collisionvehicle);
	installHook("human_grabbing_hook", human_grabbing_hook, (void*)human_grabbing, (void*)h_human_grabbing);
	installHook("grenadeexplosion_hook", grenadeexplosion_hook, (void*)grenadeexplosion, (void*)h_grenadeexplosion);
	installHook("server_playermessage_hook", server_playermessage_hook, (void*)server_playermessage, (void*)h_server_playermessage);
	installHook("playerai_hook", playerai_hook, (void*)playerai, (void*)h_playerai);
	installHook("playerdeathtax_hook", playerdeathtax_hook, (void*)playerdeathtax, (void*)h_playerdeathtax);
	installHook("addcollision_rigidbody_rigidbody_hook", addcollision_rigidbody_rigidbody_hook, (void*)addcollision_rigidbody_rigidbody, (void*)h_addcollision_rigidbody_rigidbody);

	installHook("createplayer_hook", createplayer_hook, (void*)createplayer, (void*)h_createplayer);
	installHook("deleteplayer_hook", deleteplayer_hook, (void*)deleteplayer, (void*)h_deleteplayer);
	installHook("createhuman_hook", createhuman_hook, (void*)createhuman, (void*)h_createhuman);
	installHook("deletehuman_hook", deletehuman_hook, (void*)deletehuman, (void*)h_deletehuman);
	installHook("createitem_hook", createitem_hook, (void*)createitem, (void*)h_createitem);
	installHook("deleteitem_hook", deleteitem_hook, (void*)deleteitem, (void*)h_deleteitem);
	installHook("createobject_hook", createobject_hook, (void*)createobject, (void*)h_createobject);
	installHook("deleteobject_hook", deleteobject_hook, (void*)deleteobject, (void*)h_deleteobject);
	installHook("createrigidbody_hook", createrigidbody_hook, (void*)createrigidbody, (void*)h_createrigidbody);

	installHook("createevent_message_hook", createevent_message_hook, (void*)createevent_message, (void*)h_createevent_message);
	installHook("createevent_updateplayer_hook", createevent_updateplayer_hook, (void*)createevent_updateplayer, (void*)h_createevent_updateplayer);
	//installHook("createevent_updateplayer_finance_hook", createevent_updateplayer_finance_hook, (void*)createevent_updateplayer_finance, (void*)h_createevent_updateplayer_finance);
	//createevent_updateitem_hook.Install((void*)createevent_updateitem, (void*)h_createevent_updateitem, HOOK_FLAGS);
	installHook("createevent_updateobject_hook", createevent_updateobject_hook, (void*)createevent_updateobject, (void*)h_createevent_updateobject);
	//createevent_sound_hook.Install((void*)createevent_sound, (void*)h_createevent_sound, HOOK_FLAGS);
	installHook("createevent_bullethit_hook", createevent_bullethit_hook, (void*)createevent_bullethit, (void*)h_createevent_bullethit);

	installHook("lineintersecthuman_hook", lineintersecthuman_hook, (void*)lineintersecthuman, (void*)h_lineintersecthuman);
}

static inline void attachSignalHandler()
{
	struct sigaction action;
	action.sa_handler = Console::handleInterruptSignal;

	if (sigaction(SIGINT, &action, nullptr) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
}

static void attach()
{
	// Don't load self into future child processes
	unsetenv("LD_PRELOAD");

	attachSignalHandler();

	Console::log(RS_PREFIX "Assuming 37c\n");

	Console::log(RS_PREFIX "Locating memory...\n");
	auto base = getBaseAddress();
	printBaseAddress(base);
	locateMemory(base);

	Console::log(RS_PREFIX "Installing hooks...\n");
	installHooks();

	Console::log(RS_PREFIX "Waiting for engine init...\n");
}

int __attribute__((constructor)) Entry()
{
	std::thread mainThread(attach);
	mainThread.detach();
	return 0;
}

int __attribute__((destructor)) Destroy()
{
	if (lua != nullptr)
	{
		delete lua;
		lua = nullptr;
	}
	return 0;
}