#include "RosaServer.h"
#include <sys/mman.h>
#include <cerrno>

#define handle_error(msg)               \
	do                                    \
	{                                     \
		std::cout << __LINE__ << std::endl; \
		perror(msg);                        \
		exit(EXIT_FAILURE);                 \
	} while (0)

bool initialized = false;
bool shouldReset = false;

sol::state* lua;
std::string hookMode;

sol::table* playerDataTables[MAXNUMOFPLAYERS];
sol::table* humanDataTables[MAXNUMOFHUMANS];
sol::table* itemDataTables[MAXNUMOFITEMS];
sol::table* vehicleDataTables[MAXNUMOFVEHICLES];

std::queue<std::string> consoleQueue;
std::mutex consoleQueueMutex;
std::queue<LuaHTTPRequest> requestQueue;
std::mutex requestQueueMutex;
std::queue<LuaHTTPResponse> responseQueue;
std::mutex responseQueueMutex;

static unsigned int* version;
static unsigned int* subVersion;
static char* serverName;
static unsigned int* serverPort;

static int* isPassworded;
static char* password;
static int* maxPlayers;

int* gameType;
char* mapName;
char* loadedMapName;
int* gameState;
int* gameTimer;
unsigned int* sunTime;
static int* isLevelLoaded;
static float* gravity;
static float originalGravity;

RayCastResult* lineIntersectResult;

Connection* connections;
Account* accounts;
Player* players;
Human* humans;
Vehicle* vehicles;
ItemType* itemTypes;
Item* items;
Bullet* bullets;
RigidBody* bodies;
Bond* bonds;

unsigned int* numConnections;
unsigned int* numBullets;

static void pryMemory(void* address, size_t numPages)
{
	size_t pageSize = sysconf(_SC_PAGE_SIZE);

	uintptr_t page = (uintptr_t)address;
	page -= (page % pageSize);

	if (mprotect((void*)page, pageSize * numPages, PROT_WRITE | PROT_READ) == 0)
	{
		printf("[RS] Successfully pried open page at %p\n", (void*)page);
	}
	else
	{
		handle_error("mprotect");
	}
}

/*static subhook::Hook _test_hook;
typedef int(*_test_func)(int, Vector*, Vector*);
static _test_func _test;

int h__test(int a, Vector* c, Vector* d) {
	printf("test %i (%f, %f, %f) (%f, %f, %f)\n", a, c->x, c->y, c->z, d->x, d->y, d->z);
	subhook::ScopedHookRemove remove(&_test_hook);
	int ret = _test(a, c, d);
	printf("%i\n", ret);
	return ret;
}*/

subhook::Hook resetgame_hook;
void_func resetgame;

subhook::Hook logicsimulation_hook;
void_func logicsimulation;
subhook::Hook logicsimulation_race_hook;
void_func logicsimulation_race;
subhook::Hook logicsimulation_round_hook;
void_func logicsimulation_round;
subhook::Hook logicsimulation_world_hook;
void_func logicsimulation_world;
subhook::Hook logicsimulation_terminator_hook;
void_func logicsimulation_terminator;
subhook::Hook logicsimulation_coop_hook;
void_func logicsimulation_coop;
subhook::Hook logicsimulation_versus_hook;
void_func logicsimulation_versus;
subhook::Hook logic_playeractions_hook;
void_index_func logic_playeractions;

subhook::Hook physicssimulation_hook;
void_func physicssimulation;
subhook::Hook serverrecv_hook;
serverrecv_func serverrecv;
subhook::Hook serversend_hook;
void_func serversend;
subhook::Hook bulletsimulation_hook;
void_func bulletsimulation;
void_func bullettimetolive;

subhook::Hook saveaccountsserver_hook;
void_func saveaccountsserver;

subhook::Hook createaccount_jointicket_hook;
createaccount_jointicket_func createaccount_jointicket;
// Alex Austin's typo
subhook::Hook server_sendconnectreponse_hook;
server_sendconnectreponse_func server_sendconnectreponse;

scenario_armhuman_func scenario_armhuman;
subhook::Hook linkitem_hook;
linkitem_func linkitem;
item_setmemo_func item_setmemo;
item_computertransmitline_func item_computertransmitline;
subhook::Hook item_computerinput_hook;
item_computerinput_func item_computerinput;
subhook::Hook human_applydamage_hook;
human_applydamage_func human_applydamage;
subhook::Hook human_collisionvehicle_hook;
human_collisionvehicle_func human_collisionvehicle;
subhook::Hook human_grabbing_hook;
void_index_func human_grabbing;
subhook::Hook grenadeexplosion_hook;
void_index_func grenadeexplosion;
subhook::Hook server_playermessage_hook;
server_playermessage_func server_playermessage;
subhook::Hook playerai_hook;
void_index_func playerai;
subhook::Hook playerdeathtax_hook;
void_index_func playerdeathtax;
createbond_rigidbody_rigidbody_func createbond_rigidbody_rigidbody;
createbond_rigidbody_rot_rigidbody_func createbond_rigidbody_rot_rigidbody;
createbond_rigidbody_level_func createbond_rigidbody_level;

subhook::Hook createplayer_hook;
createplayer_func createplayer;
subhook::Hook deleteplayer_hook;
void_index_func deleteplayer;
subhook::Hook createhuman_hook;
createhuman_func createhuman;
subhook::Hook deletehuman_hook;
void_index_func deletehuman;
subhook::Hook createitem_hook;
createitem_func createitem;
subhook::Hook deleteitem_hook;
void_index_func deleteitem;
createrope_func createrope;
subhook::Hook createobject_hook;
createobject_func createobject;
subhook::Hook deleteobject_hook;
void_index_func deleteobject;

subhook::Hook createevent_message_hook;
createevent_message_func createevent_message;
subhook::Hook createevent_updateplayer_hook;
void_index_func createevent_updateplayer;
subhook::Hook createevent_updateplayer_finance_hook;
void_index_func createevent_updateplayer_finance;
//subhook::Hook createevent_updateitem_hook;
//void_index_func createevent_updateitem;
void_index_func createevent_createobject;
subhook::Hook createevent_updateobject_hook;
createevent_updateobject_func createevent_updateobject;
//subhook::Hook createevent_sound_hook;
createevent_sound_func createevent_sound;
createevent_explosion_func createevent_explosion;
subhook::Hook createevent_bullethit_hook;
createevent_bullethit_func createevent_bullethit;

lineintersectlevel_func lineintersectlevel;
subhook::Hook lineintersecthuman_hook;
lineintersecthuman_func lineintersecthuman;
lineintersectobject_func lineintersectobject;

#define HOOK_FLAGS subhook::HookFlags::HookFlag64BitOffset

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

void luaInit(bool redo)
{
	printf("\033[36m");
	if (redo)
	{
		printf("\n[RS] Resetting state...\n");
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

		delete lua;
	}
	else
	{
		printf("\n[RS] Initializing state...\n");
	}

	lua = new sol::state();
	lua->open_libraries(sol::lib::base);
	lua->open_libraries(sol::lib::package);
	lua->open_libraries(sol::lib::coroutine);
	lua->open_libraries(sol::lib::string);
	lua->open_libraries(sol::lib::os);
	lua->open_libraries(sol::lib::math);
	lua->open_libraries(sol::lib::table);
	lua->open_libraries(sol::lib::debug);
	lua->open_libraries(sol::lib::bit32);
	lua->open_libraries(sol::lib::io);
	lua->open_libraries(sol::lib::ffi);
	lua->open_libraries(sol::lib::jit);

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
		auto meta = lua->new_usertype<Vector>("new", sol::no_constructor);
		meta["x"] = &Vector::x;
		meta["y"] = &Vector::y;
		meta["z"] = &Vector::z;

		meta["class"] = sol::property(&Vector::getClass);
		meta["__tostring"] = &Vector::__tostring;
		meta["add"] = &Vector::add;
		meta["mult"] = &Vector::mult;
		meta["set"] = &Vector::set;
		meta["clone"] = &Vector::clone;
		meta["dist"] = &Vector::dist;
		meta["distSquare"] = &Vector::distSquare;
	}

	{
		auto meta = lua->new_usertype<RotMatrix>("new", sol::no_constructor);
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
		meta["set"] = &RotMatrix::set;
		meta["clone"] = &RotMatrix::clone;
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
		meta["human"] = sol::property(&Player::getHuman);
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
		meta["player"] = sol::property(&Human::getPlayer);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);
		meta["rightHandItem"] = sol::property(&Human::getRightHandItem);
		meta["leftHandItem"] = sol::property(&Human::getLeftHandItem);
		meta["rightHandGrab"] = sol::property(&Human::getRightHandGrab, &Human::setRightHandGrab);
		meta["leftHandGrab"] = sol::property(&Human::getLeftHandGrab, &Human::setLeftHandGrab);

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

		meta["class"] = sol::property(&ItemType::getClass);
		meta["__tostring"] = &ItemType::__tostring;
		meta["index"] = sol::property(&ItemType::getIndex);
		meta["name"] = sol::property(&ItemType::getName, &ItemType::setName);
		meta["isGun"] = sol::property(&ItemType::getIsGun, &ItemType::setIsGun);
	}

	{
		auto meta = lua->new_usertype<Item>("new", sol::no_constructor);
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
		meta["isSettled"] = sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled);

		meta["bondTo"] = &RigidBody::bondTo;
		meta["bondRotTo"] = &RigidBody::bondRotTo;
		meta["bondToLevel"] = &RigidBody::bondToLevel;
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

	(*lua)["printAppend"] = l_printAppend;
	(*lua)["flagStateForReset"] = l_flagStateForReset;

	(*lua)["hook"] = lua->create_table();
	(*lua)["hook"]["persistentMode"] = hookMode;

	(*lua)["Vector"] = sol::overload(l_Vector, l_Vector_3f);
	(*lua)["RotMatrix"] = l_RotMatrix;

	(*lua)["http"] = lua->create_table();
	(*lua)["http"]["get"] = l_http_get;
	(*lua)["http"]["post"] = l_http_post;

	(*lua)["event"] = lua->create_table();
	(*lua)["event"]["sound"] = sol::overload(l_event_sound, l_event_soundSimple);
	(*lua)["event"]["explosion"] = l_event_explosion;
	(*lua)["event"]["bulletHit"] = l_event_bulletHit;

	(*lua)["physics"] = lua->create_table();
	(*lua)["physics"]["lineIntersectLevel"] = l_physics_lineIntersectLevel;
	(*lua)["physics"]["lineIntersectHuman"] = l_physics_lineIntersectHuman;
	(*lua)["physics"]["lineIntersectVehicle"] = l_physics_lineIntersectVehicle;
	(*lua)["physics"]["garbageCollectBullets"] = l_physics_garbageCollectBullets;

	(*lua)["chat"] = lua->create_table();
	(*lua)["chat"]["announce"] = l_chat_announce;
	(*lua)["chat"]["tellAdmins"] = l_chat_tellAdmins;
	(*lua)["chat"]["addRaw"] = l_chat_addRaw;

	(*lua)["accounts"] = lua->create_table();
	(*lua)["accounts"]["save"] = l_accounts_save;
	(*lua)["accounts"]["getCount"] = l_accounts_getCount;
	(*lua)["accounts"]["getAll"] = l_accounts_getAll;
	(*lua)["accounts"]["getByPhone"] = l_accounts_getByPhone;
	{
		sol::table _meta = lua->create_table();
		(*lua)["accounts"][sol::metatable_key] = _meta;
		_meta["__index"] = l_accounts_getByIndex;
	}

	(*lua)["players"] = lua->create_table();
	(*lua)["players"]["getCount"] = l_players_getCount;
	(*lua)["players"]["getAll"] = l_players_getAll;
	(*lua)["players"]["getByPhone"] = l_players_getByPhone;
	(*lua)["players"]["getNonBots"] = l_players_getNonBots;
	(*lua)["players"]["createBot"] = l_players_createBot;
	{
		sol::table _meta = lua->create_table();
		(*lua)["players"][sol::metatable_key] = _meta;
		_meta["__index"] = l_players_getByIndex;
	}

	(*lua)["humans"] = lua->create_table();
	(*lua)["humans"]["getCount"] = l_humans_getCount;
	(*lua)["humans"]["getAll"] = l_humans_getAll;
	(*lua)["humans"]["create"] = l_humans_create;
	{
		sol::table _meta = lua->create_table();
		(*lua)["humans"][sol::metatable_key] = _meta;
		_meta["__index"] = l_humans_getByIndex;
	}

	(*lua)["itemTypes"] = lua->create_table();
	(*lua)["itemTypes"]["getCount"] = l_itemTypes_getCount;
	(*lua)["itemTypes"]["getAll"] = l_itemTypes_getAll;
	{
		sol::table _meta = lua->create_table();
		(*lua)["itemTypes"][sol::metatable_key] = _meta;
		_meta["__index"] = l_itemTypes_getByIndex;
	}

	(*lua)["items"] = lua->create_table();
	(*lua)["items"]["getCount"] = l_items_getCount;
	(*lua)["items"]["getAll"] = l_items_getAll;
	(*lua)["items"]["create"] = sol::overload(l_items_create, l_items_createVel);
	(*lua)["items"]["createRope"] = l_items_createRope;
	{
		sol::table _meta = lua->create_table();
		(*lua)["items"][sol::metatable_key] = _meta;
		_meta["__index"] = l_items_getByIndex;
	}

	(*lua)["vehicles"] = lua->create_table();
	(*lua)["vehicles"]["getCount"] = l_vehicles_getCount;
	(*lua)["vehicles"]["getAll"] = l_vehicles_getAll;
	(*lua)["vehicles"]["create"] = sol::overload(l_vehicles_create, l_vehicles_createVel);
	//(*lua)["vehicles"]["createTraffic"] = l_vehicles_createTraffic;
	{
		sol::table _meta = lua->create_table();
		(*lua)["vehicles"][sol::metatable_key] = _meta;
		_meta["__index"] = l_vehicles_getByIndex;
	}

	(*lua)["bullets"] = lua->create_table();
	(*lua)["bullets"]["getCount"] = l_bullets_getCount;
	(*lua)["bullets"]["getAll"] = l_bullets_getAll;

	(*lua)["rigidBodies"] = lua->create_table();
	(*lua)["rigidBodies"]["getCount"] = l_rigidBodies_getCount;
	(*lua)["rigidBodies"]["getAll"] = l_rigidBodies_getAll;
	{
		sol::table _meta = lua->create_table();
		(*lua)["rigidBodies"][sol::metatable_key] = _meta;
		_meta["__index"] = l_rigidBodies_getByIndex;
	}

	(*lua)["bonds"] = lua->create_table();
	(*lua)["bonds"]["getCount"] = l_bonds_getCount;
	(*lua)["bonds"]["getAll"] = l_bonds_getAll;
	{
		sol::table _meta = lua->create_table();
		(*lua)["bonds"][sol::metatable_key] = _meta;
		_meta["__index"] = l_bonds_getByIndex;
	}

	//(*lua)["os"]["setClipboard"] = l_os_setClipboard;
	(*lua)["os"]["listDirectory"] = l_os_listDirectory;
	(*lua)["os"]["clock"] = l_os_clock;

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

	printf("[RS] Running init.lua...\033[0m\n");

	sol::load_result load = lua->load_file("main/init.lua");
	if (noLuaCallError(&load))
	{
		sol::protected_function_result res = load();
		if (noLuaCallError(&res))
		{
			printf("\033[32m[RS] Ready!\033[0m\n");
		}
	}
}

static void Attach()
{
	printf("[RS] Assuming 37c...\n");

	std::ifstream file("/proc/self/maps");
	std::string line;
	// First line
	std::getline(file, line);
	auto pos = line.find("-");
	auto truncated = line.substr(0, pos);

	printf("[RS] Base address is 0x%s...\n", truncated.c_str());

	auto base = std::stoul(truncated, nullptr, 16);

	// Locate everything

	version = (unsigned int*)(base + 0x2D5F08);
	subVersion = (unsigned int*)(base + 0x2D5F04);
	serverName = (char*)(base + 0x24EE4234);
	serverPort = (unsigned int*)(base + 0x24EE4640);
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

	numConnections = (unsigned int*)(base + 0x4532F468);
	numBullets = (unsigned int*)(base + 0x4532F240);

	//_test = (_test_func)(base + 0x12B80);
	//pryMemory(&_test, 2);

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

	createplayer = (createplayer_func)(base + 0x40EE0);
	deleteplayer = (void_index_func)(base + 0x411D0);
	createhuman = (createhuman_func)(base + 0x66D10);
	deletehuman = (void_index_func)(base + 0x3EB0);
	createitem = (createitem_func)(base + 0x4DDE0);
	deleteitem = (void_index_func)(base + 0x2C180);
	createrope = (createrope_func)(base + 0x4F150);
	createobject = (createobject_func)(base + 0x4CEA0);
	deleteobject = (void_index_func)(base + 0x42A0);

	createevent_message = (createevent_message_func)(base + 0x29C0);
	createevent_updateplayer = (void_index_func)(base + 0x2BE0);
	createevent_updateplayer_finance = (void_index_func)(base + 0x2D00);
	//pryMemory(&createevent_updateplayer_finance, 2);
	//createevent_updateitem = (void_index_func)(base + 0x27B0);//
	createevent_createobject = (void_index_func)(base + 0x2AE0);
	createevent_updateobject = (createevent_updateobject_func)(base + 0x41C0);
	createevent_sound = (createevent_sound_func)(base + 0x3CC0);
	createevent_explosion = (createevent_explosion_func)(base + 0x45A0);
	createevent_bullethit = (createevent_bullethit_func)(base + 0x4110);

	lineintersecthuman = (lineintersecthuman_func)(base + 0x23AB0);
	lineintersectlevel = (lineintersectlevel_func)(base + 0x7C470);
	lineintersectobject = (lineintersectobject_func)(base + 0x95590);

	// Hooks

	//_test_hook.Install((void*)_test, (void*)h__test, HOOK_FLAGS);
	resetgame_hook.Install((void*)resetgame, (void*)h_resetgame, HOOK_FLAGS);

	logicsimulation_hook.Install((void*)logicsimulation, (void*)h_logicsimulation, HOOK_FLAGS);
	logicsimulation_race_hook.Install((void*)logicsimulation_race, (void*)h_logicsimulation_race, HOOK_FLAGS);
	logicsimulation_round_hook.Install((void*)logicsimulation_round, (void*)h_logicsimulation_round, HOOK_FLAGS);
	logicsimulation_world_hook.Install((void*)logicsimulation_world, (void*)h_logicsimulation_world, HOOK_FLAGS);
	logicsimulation_terminator_hook.Install((void*)logicsimulation_terminator, (void*)h_logicsimulation_terminator, HOOK_FLAGS);
	logicsimulation_coop_hook.Install((void*)logicsimulation_coop, (void*)h_logicsimulation_coop, HOOK_FLAGS);
	logicsimulation_versus_hook.Install((void*)logicsimulation_versus, (void*)h_logicsimulation_versus, HOOK_FLAGS);
	logic_playeractions_hook.Install((void*)logic_playeractions, (void*)h_logic_playeractions, HOOK_FLAGS);

	physicssimulation_hook.Install((void*)physicssimulation, (void*)h_physicssimulation, HOOK_FLAGS);
	serverrecv_hook.Install((void*)serverrecv, (void*)h_serverrecv, HOOK_FLAGS);
	serversend_hook.Install((void*)serversend, (void*)h_serversend, HOOK_FLAGS);
	bulletsimulation_hook.Install((void*)bulletsimulation, (void*)h_bulletsimulation, HOOK_FLAGS);

	saveaccountsserver_hook.Install((void*)saveaccountsserver, (void*)h_saveaccountsserver, HOOK_FLAGS);

	createaccount_jointicket_hook.Install((void*)createaccount_jointicket, (void*)h_createaccount_jointicket, HOOK_FLAGS);
	server_sendconnectreponse_hook.Install((void*)server_sendconnectreponse, (void*)h_server_sendconnectreponse, HOOK_FLAGS);

	linkitem_hook.Install((void*)linkitem, (void*)h_linkitem, HOOK_FLAGS);
	item_computerinput_hook.Install((void*)item_computerinput, (void*)h_item_computerinput, HOOK_FLAGS);
	human_applydamage_hook.Install((void*)human_applydamage, (void*)h_human_applydamage, HOOK_FLAGS);
	human_collisionvehicle_hook.Install((void*)human_collisionvehicle, (void*)h_human_collisionvehicle, HOOK_FLAGS);
	human_grabbing_hook.Install((void*)human_grabbing, (void*)h_human_grabbing, HOOK_FLAGS);
	grenadeexplosion_hook.Install((void*)grenadeexplosion, (void*)h_grenadeexplosion, HOOK_FLAGS);
	server_playermessage_hook.Install((void*)server_playermessage, (void*)h_server_playermessage, HOOK_FLAGS);
	playerai_hook.Install((void*)playerai, (void*)h_playerai, HOOK_FLAGS);
	playerdeathtax_hook.Install((void*)playerdeathtax, (void*)h_playerdeathtax, HOOK_FLAGS);

	createplayer_hook.Install((void*)createplayer, (void*)h_createplayer, HOOK_FLAGS);
	deleteplayer_hook.Install((void*)deleteplayer, (void*)h_deleteplayer, HOOK_FLAGS);
	createhuman_hook.Install((void*)createhuman, (void*)h_createhuman, HOOK_FLAGS);
	deletehuman_hook.Install((void*)deletehuman, (void*)h_deletehuman, HOOK_FLAGS);
	createitem_hook.Install((void*)createitem, (void*)h_createitem, HOOK_FLAGS);
	deleteitem_hook.Install((void*)deleteitem, (void*)h_deleteitem, HOOK_FLAGS);
	createobject_hook.Install((void*)createobject, (void*)h_createobject, HOOK_FLAGS);
	deleteobject_hook.Install((void*)deleteobject, (void*)h_deleteobject, HOOK_FLAGS);

	createevent_message_hook.Install((void*)createevent_message, (void*)h_createevent_message, HOOK_FLAGS);
	createevent_updateplayer_hook.Install((void*)createevent_updateplayer, (void*)h_createevent_updateplayer, HOOK_FLAGS);
	//createevent_updateplayer_finance_hook.Install((void*)createevent_updateplayer_finance, (void*)h_createevent_updateplayer_finance, HOOK_FLAGS);
	//createevent_updateitem_hook.Install((void*)createevent_updateitem, (void*)h_createevent_updateitem, HOOK_FLAGS);
	createevent_updateobject_hook.Install((void*)createevent_updateobject, (void*)h_createevent_updateobject, HOOK_FLAGS);
	//createevent_sound_hook.Install((void*)createevent_sound, (void*)h_createevent_sound, HOOK_FLAGS);
	createevent_bullethit_hook.Install((void*)createevent_bullethit, (void*)h_createevent_bullethit, HOOK_FLAGS);

	lineintersecthuman_hook.Install((void*)lineintersecthuman, (void*)h_lineintersecthuman, HOOK_FLAGS);
}

int __attribute__((constructor)) Entry()
{
	std::thread mainThread(Attach);
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