#include "pch.h"

#include "api.h"
#include "hooks.h"

HMODULE DllHandle;

bool initialized = false;
bool shouldReset = false;

sol::state* lua;
std::string hookMode = "";

std::queue<std::string> consoleQueue;
std::queue<LuaHTTPRequest> requestQueue;

static Version* version;
static char* serverName;
static unsigned int* serverPort;

static BOOL* isPassworded;
static char* password;

int* gameType;
char* mapName;
int* gameState;
int* gameTimer;

static BOOL* isLevelLoaded;

RayCastResult* lineIntersectResult;

unsigned int* sunTime;

Connection* connections;
Account* accounts;
Player* players;
Human* humans;
Vehicle* vehicles;
ItemType* itemTypes;
Item* items;
Bullet* bullets;
RigidBody* bodies;

unsigned int* numConnections;
unsigned int* numBullets;

playerai_func playerai;
recvpacket_func recvpacket;
void_func rigidbodysimulation;
void_func objectsimulation;
void_func itemsimulation;
void_func humansimulation;
void_func logicsimulation;
void_func logicsimulation_race;
void_func logicsimulation_round;
void_func logicsimulation_world;
void_func logicsimulation_terminator;
void_func logicsimulation_coop;
void_func logicsimulation_versus;
void_func sendpacket;
void_func bulletsimulation;
void_func bullettimetolive;
void_func resetgame;
void_index_func scenario_createtraffic3;
armhuman_func scenario_armhuman;
grabitem_func linkitem;
chat_func chat;
void_func createlevel;
createplayer_func createplayer;
void_index_func deleteplayer;
void_index_func playerdeathtax;
createhuman_func createhuman;
void_index_func deletehuman;
human_applydamage_func human_applydamage;
createitem_func createitem;
void_index_func deleteitem;
createrope_func createrope;
createvehicle_func createvehicle;
void_index_func deleteobject;
void_index_func grenadeexplosion;
createevent_message_func createevent_message;
void_index_func createevent_updateplayer;
void_index_func createevent_updateplayer_finance;
void_index_func createevent_updatehuman;
void_index_func createevent_updateitem;
void_index_func createevent_createobject;
createevent_updateobject_func createevent_updateobject;
createevent_sound_func createevent_sound;
createevent_explosion_func createevent_explosion;
createevent_updatedoor_func createevent_updatedoor;
createevent_bullethit_func createevent_bullethit;
lineintersectlevel_func lineintersectlevel;
lineintersecthuman_func lineintersecthuman;
lineintersectobject_func lineintersectobject;

#pragma warning( push )
#pragma warning( disable : 26444 )

struct Server {
	const int TPS = 60;

	int getPort() const {
		return *serverPort;
	}
	char* getName() const {
		return serverName;
	}
	void setName(const char* newName) const {
		strncpy(serverName, newName, 31);
	}
	char* getPassword() const {
		return password;
	}
	void setPassword(const char* newPassword) const {
		strncpy(password, newPassword, 31);
		*isPassworded = newPassword[0] != 0;
	}
	int getType() const {
		return *gameType;
	}
	void setType(int type) const {
		*gameType = type;
	}
	char* getLevelName() const {
		return mapName;
	}
	void setLevelName(const char* newName) const {
		strncpy(mapName, newName, 31);
	}
	bool getIsLevelLoaded() const {
		return *isLevelLoaded;
	}
	void setIsLevelLoaded(bool b) const {
		*isLevelLoaded = b;
	}
	int getState() const {
		return *gameState;
	}
	void setState(int state) const {
		*gameState = state;
	}
	int getTime() const {
		return *gameTimer;
	}
	void setTime(int time) const {
		*gameTimer = time;
	}
	int getSunTime() const {
		return *sunTime;
	}
	void setSunTime(int time) const {
		*sunTime = time % 5184000;
	}
	std::string getVersion() const {
		std::ostringstream stream;
		stream << version->major << (char)(version->build + 97);
		return stream.str();
	}
	const char* getConsoleTitle() const {
		LPSTR buffer = "";
		DWORD size = 0;
		GetConsoleTitleA(buffer, size);
		return buffer;
	}
	void setConsoleTitle(const char* title) const {
		SetConsoleTitleA((char*)title);
	}

	static void reset() {
		hookAndReset(RESET_REASON_LUACALL);
	}
};
static Server* server;

void luaInit(bool redo) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	if (redo) {
		printf("\n[Lua] Resetting state...\n");
		delete server;
		delete lua;
	}
	else {
		printf("\n[Lua] Initializing state...\n");
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

	lua->new_usertype<Server>("Server",
		"new", sol::no_constructor,
		"TPS", &Server::TPS,

		"port", sol::property(&Server::getPort),
		"name", sol::property(&Server::getName, &Server::setName),
		"password", sol::property(&Server::getPassword, &Server::setPassword),
		"type", sol::property(&Server::getType, &Server::setType),
		"level", sol::property(&Server::getLevelName, &Server::setLevelName),
		"isLevelLoaded", sol::property(&Server::getIsLevelLoaded, &Server::setIsLevelLoaded),
		"state", sol::property(&Server::getState, &Server::setState),
		"time", sol::property(&Server::getTime, &Server::setTime),
		"sunTime", sol::property(&Server::getSunTime, &Server::setSunTime),
		"version", sol::property(&Server::getVersion),
		"consoleTitle", sol::property(&Server::getConsoleTitle, &Server::setConsoleTitle),

		"reset", &Server::reset
	);

	server = new Server();
	(*lua)["server"] = server;

	lua->new_usertype<Connection>("Connection",
		"new", sol::no_constructor,
		"port", &Connection::port,
		"timeoutTime", &Connection::timeoutTime,

		"address", sol::property(&Connection::getAddress),
		"adminVisible", sol::property(&Connection::getAdminVisible, &Connection::setAdminVisible)
	);

	lua->new_usertype<Account>("Account",
		"new", sol::no_constructor,
		"subRosaID", &Account::subRosaID,
		"phoneNumber", &Account::phoneNumber,
		"money", &Account::money,
		"banTime", &Account::banTime,
		"name", sol::property(&Account::getName),
		"steamID", sol::property(&Account::getSteamID)
	);

	lua->new_usertype<Vector>("Vector",
		"new", sol::no_constructor,
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z,

		"add", &Vector::add,
		"mult", &Vector::mult,
		"set", &Vector::set,
		"clone", &Vector::clone,
		"dist", &Vector::dist,
		"distSquare", &Vector::distSquare
	);

	lua->new_usertype<RotMatrix>("RotMatrix",
		"new", sol::no_constructor,
		"x1", &RotMatrix::x1,
		"y1", &RotMatrix::y1,
		"z1", &RotMatrix::z1,
		"x2", &RotMatrix::x2,
		"y2", &RotMatrix::y2,
		"z2", &RotMatrix::z2,
		"x3", &RotMatrix::x3,
		"y3", &RotMatrix::y3,
		"z3", &RotMatrix::z3,

		"set", &RotMatrix::set,
		"clone", &RotMatrix::clone
	);

	lua->new_usertype<Player>("Player",
		"new", sol::no_constructor,
		"subRosaID", &Player::subRosaID,
		"phoneNumber", &Player::phoneNumber,
		"accountID", &Player::accountID,
		"money", &Player::money,
		"team", &Player::team,
		"teamSwitchTimer", &Player::teamSwitchTimer,
		"stocks", &Player::stocks,
		"humanID", &Player::humanID,
		"menuTab", &Player::menuTab,
		"gender", &Player::gender,
		"skinColor", &Player::skinColor,
		"hairColor", &Player::hairColor,
		"hair", &Player::hair,
		"eyeColor", &Player::eyeColor,
		"shirtColor", &Player::shirtColor,
		"suitColor", &Player::suitColor,
		"tieColor", &Player::tieColor,
		"head", &Player::head,
		"necklace", &Player::necklace,

		"index", sol::property(&Player::getIndex),
		"isActive", sol::property(&Player::getIsActive, &Player::setIsActive),
		"name", sol::property(&Player::getName, &Player::setName),
		"isAdmin", sol::property(&Player::getIsAdmin, &Player::setIsAdmin),
		"isReady", sol::property(&Player::getIsReady, &Player::setIsReady),
		"isBot", sol::property(&Player::getIsBot, &Player::setIsBot),
		"human", sol::property(&Player::getHuman),
		"connection", sol::property(&Player::getConnection),

		"update", &Player::update,
		"updateFinance", &Player::updateFinance,
		"remove", &Player::remove
	);

	lua->new_usertype<Human>("Human",
		"new", sol::no_constructor,
		"vehicleSeat", &Human::vehicleSeat,
		"despawnTime", &Human::despawnTime,
		"damage", &Human::damage,
		"viewYaw", &Human::viewYaw,
		"viewPitch", &Human::viewPitch,
		"strafeInput", &Human::strafeInput,
		"walkInput", &Human::walkInput,
		"inputFlags", &Human::inputFlags,
		"lastInputFlags", &Human::lastInputFlags,
		"health", &Human::health,
		"bloodLevel", &Human::bloodLevel,
		"chestHP", &Human::chestHP,
		"headHP", &Human::headHP,
		"leftArmHP", &Human::leftArmHP,
		"rightArmHP", &Human::rightArmHP,
		"leftLegHP", &Human::leftLegHP,
		"rightLegHP", &Human::rightLegHP,
		"gender", &Human::gender,
		"head", &Human::head,
		"skinColor", &Human::skinColor,
		"hairColor", &Human::hairColor,
		"hair", &Human::hair,
		"eyeColor", &Human::eyeColor,

		"index", sol::property(&Human::getIndex),
		"isActive", sol::property(&Human::getIsActive, &Human::setIsActive),
		"isAlive", sol::property(&Human::getIsAlive, &Human::setIsAlive),
		"isImmortal", sol::property(&Human::getIsImmortal, &Human::setIsImmortal),
		"isOnGround", sol::property(&Human::getIsOnGround),
		"isStanding", sol::property(&Human::getIsStanding),
		"isBleeding", sol::property(&Human::getIsBleeding, &Human::setIsBleeding),
		"player", sol::property(&Human::getPlayer),
		"vehicle", sol::property(&Human::getVehicle, &Human::setVehicle),

		"update", &Human::update,
		"remove", &Human::remove,
		"getPos", &Human::getPos,
		"setPos", &Human::setPos,
		"speak", &Human::speak,
		"arm", &Human::arm,
		"getBone", &Human::getBone,
		"getRigidBody", &Human::getRigidBody,
		"setVelocity", &Human::setVelocity,
		"addVelocity", &Human::addVelocity,
		"mountItem", &Human::mountItem,
		"applyDamage", &Human::applyDamage
	);

	lua->new_usertype<ItemType>("ItemType",
		"new", sol::no_constructor,
		"price", &ItemType::price,
		"mass", &ItemType::mass,
		"fireRate", &ItemType::fireRate,
		"bulletType", &ItemType::bulletType,
		"bulletVelocity", &ItemType::bulletVelocity,
		"bulletSpread", &ItemType::bulletSpread,

		"index", sol::property(&ItemType::getIndex),
		"name", sol::property(&ItemType::getName, &ItemType::setName),
		"isGun", sol::property(&ItemType::getIsGun, &ItemType::setIsGun)
	);

	lua->new_usertype<Item>("Item",
		"new", sol::no_constructor,
		"type", &Item::type,
		"parentSlot", &Item::parentSlot,
		"pos", &Item::pos,
		"vel", &Item::vel,
		"rot", &Item::rot,
		"bullets", &Item::bullets,

		"index", sol::property(&Item::getIndex),
		"isActive", sol::property(&Item::getIsActive, &Item::setIsActive),
		"hasPhysics", sol::property(&Item::getHasPhysics, &Item::setHasPhysics),
		"physicsSettled", sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled),
		"rigidBody", sol::property(&Item::getRigidBody),

		"update", &Item::update,
		"remove", &Item::remove,
		"getParentHuman", &Item::getParentHuman,
		"getParentItem", &Item::getParentItem,
		"mountItem", &Item::mountItem,
		"speak", &Item::speak,
		"explode", &Item::explode
	);

	lua->new_usertype<Vehicle>("Vehicle",
		"new", sol::no_constructor,
		"type", &Vehicle::type,
		"controllableState", &Vehicle::controllableState,
		"health", &Vehicle::health,
		"lastDriverPlayerID", &Vehicle::lastDriverPlayerID,
		"color", &Vehicle::color,
		"pos", &Vehicle::pos,
		"pos2", &Vehicle::pos2,
		"rot", &Vehicle::rot,
		"vel", &Vehicle::vel,
		"windowState0", &Vehicle::windowState0,
		"windowState1", &Vehicle::windowState1,
		"windowState2", &Vehicle::windowState2,
		"windowState3", &Vehicle::windowState3,
		"windowState4", &Vehicle::windowState4,
		"windowState5", &Vehicle::windowState5,
		"windowState6", &Vehicle::windowState6,
		"windowState7", &Vehicle::windowState7,
		"gearX", &Vehicle::gearX,
		"steerControl", &Vehicle::steerControl,
		"gearY", &Vehicle::gearY,
		"gasControl", &Vehicle::gasControl,
		"numWheels", &Vehicle::numWheels,
		"wheelBodyID0", &Vehicle::wheelBodyID0,
		"wheelBodyID1", &Vehicle::wheelBodyID1,
		"wheelBodyID2", &Vehicle::wheelBodyID2,
		"wheelBodyID3", &Vehicle::wheelBodyID3,
		"bladeBodyID", &Vehicle::bladeBodyID,

		"index", sol::property(&Vehicle::getIndex),
		"isActive", sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive),
		"rigidBody", sol::property(&Vehicle::getRigidBody),

		"updateType", &Vehicle::updateType,
		"updateDestruction", &Vehicle::updateDestruction,
		"remove", &Vehicle::remove
	);

	lua->new_usertype<Bullet>("Bullet",
		"new", sol::no_constructor,
		"type", &Bullet::type,
		"time", &Bullet::time,
		"lastPos", &Bullet::lastPos,
		"pos", &Bullet::pos,
		"vel", &Bullet::vel,

		"player", sol::property(&Bullet::getPlayer)
	);

	lua->new_usertype<Bone>("Bone",
		"new", sol::no_constructor,
		"pos", &Bone::pos,
		"pos2", &Bone::pos2
	);

	lua->new_usertype<RigidBody>("RigidBody",
		"new", sol::no_constructor,
		"type", &RigidBody::type,
		"unk0", &RigidBody::unk0,
		"mass", &RigidBody::mass,
		"pos", &RigidBody::pos,
		"vel", &RigidBody::vel,
		"rot", &RigidBody::rot,
		"rot2", &RigidBody::rot2,

		"index", sol::property(&RigidBody::getIndex),
		"isActive", sol::property(&RigidBody::getIsActive, &RigidBody::setIsActive),
		"isSettled", sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled)
	);

	(*lua)["printAppend"] = l_printAppend;
	(*lua)["flagStateForReset"] = l_flagStateForReset;

	(*lua)["hook"] = lua->create_table();
	(*lua)["hook"]["persistentMode"] = hookMode;

	(*lua)["Vector"] = sol::overload(l_Vector, l_Vector_3f);
	(*lua)["RotMatrix"] = l_RotMatrix;

	(*lua)["http"] = lua->create_table();
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
	(*lua)["accounts"]["getAll"] = l_accounts_getAll;
	(*lua)["accounts"]["getByPhone"] = l_accounts_getByPhone;

	(*lua)["players"] = lua->create_table();
	(*lua)["players"]["getAll"] = l_players_getAll;
	(*lua)["players"]["getByPhone"] = l_players_getByPhone;
	(*lua)["players"]["getNonBots"] = l_players_getNonBots;
	(*lua)["players"]["getByIndex"] = l_players_getByIndex;
	(*lua)["players"]["createBot"] = l_players_createBot;

	(*lua)["humans"] = lua->create_table();
	(*lua)["humans"]["getAll"] = l_humans_getAll;
	(*lua)["humans"]["getByIndex"] = l_humans_getByIndex;
	(*lua)["humans"]["create"] = l_humans_create;

	(*lua)["itemTypes"] = lua->create_table();
	(*lua)["itemTypes"]["getAll"] = l_itemTypes_getAll;
	(*lua)["itemTypes"]["getByIndex"] = l_itemTypes_getByIndex;

	(*lua)["items"] = lua->create_table();
	(*lua)["items"]["getCount"] = l_items_getCount;
	(*lua)["items"]["getAll"] = l_items_getAll;
	(*lua)["items"]["getByIndex"] = l_items_getByIndex;
	(*lua)["items"]["create"] = sol::overload(l_items_create, l_items_createVel);
	(*lua)["items"]["createRope"] = lua_items_createRope;

	(*lua)["vehicles"] = lua->create_table();
	(*lua)["vehicles"]["getAll"] = l_vehicles_getAll;
	(*lua)["vehicles"]["getByIndex"] = l_vehicles_getByIndex;
	(*lua)["vehicles"]["create"] = sol::overload(l_vehicles_create, l_vehicles_createVel);
	(*lua)["vehicles"]["createTraffic"] = l_vehicles_createTraffic;

	(*lua)["bullets"] = lua->create_table();
	(*lua)["bullets"]["getAll"] = l_bullets_getAll;

	(*lua)["rigidBodies"] = lua->create_table();
	(*lua)["rigidBodies"]["getCount"] = l_rigidBodies_getCount;
	(*lua)["rigidBodies"]["getAll"] = l_rigidBodies_getAll;
	(*lua)["rigidBodies"]["getByIndex"] = l_rigidBodies_getByIndex;

	(*lua)["os"]["setClipboard"] = l_os_setClipboard;
	(*lua)["os"]["listDirectory"] = l_os_listDirectory;

	printf("[Lua] Running main.lua...\n");
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	sol::load_result load = lua->load_file("lua/main.lua");
	if (noLuaCallError(&load)) {
		sol::protected_function_result res = load();
		if (noLuaCallError(&res)) {
			SetConsoleTextAttribute(handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			printf("[Lua] No problems.\n");
			SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
	}
}

static BOOL Init(HMODULE& hModule) {
	DllHandle = hModule;

	{
		DWORD exeBase = (DWORD)GetModuleHandle(NULL);

		//2300000003000000
		version = (Version*)(exeBase + 0x8B3F0);

		serverName = (char*)(exeBase + 0x8B12554);
		serverPort = (unsigned int*)(exeBase + 0x8ED0BC0);
		isPassworded = (BOOL*)(exeBase + 0x8b12964);
		password = (char*)(exeBase + 0x8ED11CC);
		gameType = (int*)(exeBase + 0x8ED1548);
		mapName = (char*)(exeBase + 0x8ED154C);
		gameState = (int*)(exeBase + 0x8ED1764);
		gameTimer = (int*)(exeBase + 0x8ED176C);
		isLevelLoaded = (BOOL*)(exeBase + 0x13116AE0);

		lineIntersectResult = (RayCastResult*)(exeBase + 0x192BF980);
		sunTime = (unsigned int*)(exeBase + 0x33E58A20);

		connections = (Connection*)(exeBase + 0x1DF6A800);
		accounts = (Account*)(exeBase + 0x16DFCE90);
		players = (Player*)(exeBase + 0x13841600);
		humans = (Human*)(exeBase + 0x5312c4);
		vehicles = (Vehicle*)(exeBase + 0x80f66e0);
		itemTypes = (ItemType*)(exeBase + 0x192d1560);
		items = (Item*)(exeBase + 0x8f011a0);
		bullets = (Bullet*)(exeBase + 0x1ec760);
		bodies = (RigidBody*)(exeBase + 0x8f840);

		numConnections = (unsigned int*)(exeBase + 0x917b00);
		numBullets = (unsigned int*)(exeBase + 0x26a553c0);

		playerai = (playerai_func)(exeBase + 0x72770);
		rigidbodysimulation = (void_func)(exeBase + 0xF840);
		objectsimulation = (void_func)(exeBase + 0x774A0);
		itemsimulation = (void_func)(exeBase + 0x74F00);
		humansimulation = (void_func)(exeBase + 0x7CDF0);

		logicsimulation = (void_func)(exeBase + 0x85eb0);
		logicsimulation_race = (void_func)(exeBase + 0x81950);
		logicsimulation_round = (void_func)(exeBase + 0x84700);
		logicsimulation_world = (void_func)(exeBase + 0x856B0);
		logicsimulation_terminator = (void_func)(exeBase + 0x820F0);
		logicsimulation_coop = (void_func)(exeBase + 0x81610);
		logicsimulation_versus = (void_func)(exeBase + 0x835E0);

		recvpacket = (recvpacket_func)(exeBase + 0x70CA0);
		sendpacket = (void_func)(exeBase + 0x6FFD0);
		bulletsimulation = (void_func)(exeBase + 0x5E790);
		bullettimetolive = (void_func)(exeBase + 0x19BD0);

		resetgame = (void_func)(exeBase + 0x7a680);
		scenario_createtraffic3 = (void_index_func)(exeBase + 0x6FE70);

		scenario_armhuman = (armhuman_func)(exeBase + 0x4d4b0);

		linkitem = (grabitem_func)(exeBase + 0x45120);
		chat = (chat_func)(exeBase + 0x604c0);

		createlevel = (void_func)(exeBase + 0x755C0);

		createplayer = (createplayer_func)(exeBase + 0x31b50);
		createhuman = (createhuman_func)(exeBase + 0x74760);
		createitem = (createitem_func)(exeBase + 0x44880);
		createrope = (createrope_func)(exeBase + 0x459f0);
		createvehicle = (createvehicle_func)(exeBase + 0x48860);

		playerdeathtax = (void_index_func)(exeBase + 0x1bb40);
		human_applydamage = (human_applydamage_func)(exeBase + 0x8BF0);

		deleteplayer = (void_index_func)(exeBase + 0xe880);
		deletehuman = (void_index_func)(exeBase + 0x3d470);
		deleteitem = (void_index_func)(exeBase + 0x44c10);
		deleteobject = (void_index_func)(exeBase + 0x298c0);
		grenadeexplosion = (void_index_func)(exeBase + 0x21db0);

		createevent_message = (createevent_message_func)(exeBase + 0x7450);
		createevent_updateplayer = (void_index_func)(exeBase + 0x7690);
		createevent_updateplayer_finance = (void_index_func)(exeBase + 0x7800);
		createevent_updatehuman = (void_index_func)(exeBase + 0x7790);
		createevent_updateitem = (void_index_func)(exeBase + 0x7570);
		createevent_createobject = (void_index_func)(exeBase + 0x7500);
		createevent_updateobject = (createevent_updateobject_func)(exeBase + 0x1bf60);
		createevent_sound = (createevent_sound_func)(exeBase + 0x1c030);
		createevent_explosion = (createevent_explosion_func)(exeBase + 0x1c190);
		createevent_updatedoor = (createevent_updatedoor_func)(exeBase + 0x78f0);
		createevent_bullethit = (createevent_bullethit_func)(exeBase + 0x1bec0);

		lineintersecthuman = (lineintersecthuman_func)(exeBase + 0x3d680);
		lineintersectlevel = (lineintersectlevel_func)(exeBase + 0x45e10);
		lineintersectobject = (lineintersectobject_func)(exeBase + 0x2c390);
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach((PVOID*)(&playerai), h_playerai);
	DetourAttach((PVOID*)(&rigidbodysimulation), h_rigidbodysimulation);
	DetourAttach((PVOID*)(&objectsimulation), h_objectsimulation);
	DetourAttach((PVOID*)(&itemsimulation), h_itemsimulation);
	DetourAttach((PVOID*)(&humansimulation), h_humansimulation);

	DetourAttach((PVOID*)(&logicsimulation), h_logicsimulation);
	DetourAttach((PVOID*)(&logicsimulation_race), h_logicsimulation_race);
	DetourAttach((PVOID*)(&logicsimulation_round), h_logicsimulation_round);
	DetourAttach((PVOID*)(&logicsimulation_world), h_logicsimulation_world);
	DetourAttach((PVOID*)(&logicsimulation_terminator), h_logicsimulation_terminator);
	DetourAttach((PVOID*)(&logicsimulation_coop), h_logicsimulation_coop);
	DetourAttach((PVOID*)(&logicsimulation_versus), h_logicsimulation_versus);

	DetourAttach((PVOID*)(&recvpacket), h_recvpacket);
	DetourAttach((PVOID*)(&sendpacket), h_sendpacket);
	DetourAttach((PVOID*)(&bulletsimulation), h_bulletsimulation);

	DetourAttach((PVOID*)(&resetgame), h_resetgame);
	DetourAttach((PVOID*)(&scenario_createtraffic3), h_scenario_createtraffic3);

	DetourAttach((PVOID*)(&linkitem), h_linkitem);
	DetourAttach((PVOID*)(&chat), h_chat);

	DetourAttach((PVOID*)(&createlevel), h_createlevel);

	DetourAttach((PVOID*)(&createplayer), h_createplayer);
	DetourAttach((PVOID*)(&deleteplayer), h_deleteplayer);
	DetourAttach((PVOID*)(&createhuman), h_createhuman);
	DetourAttach((PVOID*)(&deletehuman), h_deletehuman);
	DetourAttach((PVOID*)(&createitem), h_createitem);
	DetourAttach((PVOID*)(&createvehicle), h_createvehicle);
	DetourAttach((PVOID*)(&grenadeexplosion), h_grenadeexplosion);

	DetourAttach((PVOID*)(&playerdeathtax), h_playerdeathtax);
	DetourAttach((PVOID*)(&human_applydamage), h_human_applydamage);

	DetourAttach((PVOID*)(&createevent_message), h_createevent_message);
	DetourAttach((PVOID*)(&createevent_updateitem), h_createevent_updateitem);
	DetourAttach((PVOID*)(&createevent_updateplayer), h_createevent_updateplayer);
	DetourAttach((PVOID*)(&createevent_updateplayer_finance), h_createevent_updateplayer_finance);
	DetourAttach((PVOID*)(&createevent_updateobject), h_createevent_updateobject);
	DetourAttach((PVOID*)(&createevent_sound), h_createevent_sound);
	DetourAttach((PVOID*)(&createevent_updatedoor), h_createevent_updatedoor);
	DetourAttach((PVOID*)(&createevent_bullethit), h_createevent_bullethit);

	DetourTransactionCommit();
	
	return TRUE;
}

static BOOL Destroy(HMODULE& hModule) {
	if (lua != nullptr) delete lua;

	return TRUE;
}

#pragma warning( pop )

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH)
		return Init(hModule);
	if (dwReason == DLL_PROCESS_DETACH)
		return Destroy(hModule);
	return TRUE;
}

// Allows load table patching
extern "C" void __declspec(dllexport) __cdecl loader() {}