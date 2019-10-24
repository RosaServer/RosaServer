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
char* loadedMapName;
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
	char* getLoadedLevelName() const {
		return loadedMapName;
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

	{
		auto meta = lua->new_usertype<Server>("new", sol::no_constructor);
		meta["TPS"] = &Server::TPS;

		meta["port"] = sol::property(&Server::getPort);
		meta["name"] = sol::property(&Server::getName, &Server::setName);
		meta["password"] = sol::property(&Server::getPassword, &Server::setPassword);
		meta["type"] = sol::property(&Server::getType, &Server::setType);
		meta["levelToLoad"] = sol::property(&Server::getLevelName, &Server::setLevelName);
		meta["loadedLevel"] = sol::property(&Server::getLoadedLevelName);
		meta["isLevelLoaded"] = sol::property(&Server::getIsLevelLoaded, &Server::setIsLevelLoaded);
		meta["state"] = sol::property(&Server::getState, &Server::setState);
		meta["time"] = sol::property(&Server::getTime, &Server::setTime);
		meta["sunTime"] = sol::property(&Server::getSunTime, &Server::setSunTime);
		meta["version"] = sol::property(&Server::getVersion);
		meta["consoleTitle"] = sol::property(&Server::getConsoleTitle, &Server::setConsoleTitle);

		meta["reset"] = &Server::reset;
	}

	server = new Server();
	(*lua)["server"] = server;

	{
		auto meta = lua->new_usertype<Connection>("new", sol::no_constructor);
		meta["port"] = &Connection::port;
		meta["timeoutTime"] = &Connection::timeoutTime;

		meta["address"] = sol::property(&Connection::getAddress);
		meta["adminVisible"] = sol::property(&Connection::getAdminVisible, &Connection::setAdminVisible);
	}

	{
		auto meta = lua->new_usertype<Account>("new", sol::no_constructor);
		meta["subRosaID"] = &Account::subRosaID;
		meta["phoneNumber"] = &Account::phoneNumber;
		meta["money"] = &Account::money;
		meta["banTime"] = &Account::banTime;

		meta["index"] = sol::property(&Account::getIndex);
		meta["name"] = sol::property(&Account::getName);
		meta["steamID"] = sol::property(&Account::getSteamID);
	}

	{
		auto meta = lua->new_usertype<Vector>("new", sol::no_constructor);
		meta["x"] = &Vector::x;
		meta["y"] = &Vector::y;
		meta["z"] = &Vector::z;

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

		meta["set"] = &RotMatrix::set;
		meta["clone"] = &RotMatrix::clone;
	}

	{
		auto meta = lua->new_usertype<Player>("new", sol::no_constructor);
		meta["subRosaID"] = &Player::subRosaID;
		meta["phoneNumber"] = &Player::phoneNumber;
		meta["money"] = &Player::money;
		meta["team"] = &Player::team;
		meta["teamSwitchTimer"] = &Player::teamSwitchTimer;
		meta["stocks"] = &Player::stocks;
		meta["menuTab"] = &Player::menuTab;
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

		meta["index"] = sol::property(&Player::getIndex);
		meta["isActive"] = sol::property(&Player::getIsActive, &Player::setIsActive);
		meta["name"] = sol::property(&Player::getName, &Player::setName);
		meta["isAdmin"] = sol::property(&Player::getIsAdmin, &Player::setIsAdmin);
		meta["isReady"] = sol::property(&Player::getIsReady, &Player::setIsReady);
		meta["isBot"] = sol::property(&Player::getIsBot, &Player::setIsBot);
		meta["human"] = sol::property(&Player::getHuman);
		meta["connection"] = sol::property(&Player::getConnection);
		meta["account"] = sol::property(&Player::getAccount, &Player::setAccount);
		meta["botDestination"] = sol::property(&Player::getBotDestination, &Player::setBotDestination);

		meta["update"] = &Player::update;
		meta["updateFinance"] = &Player::updateFinance;
		meta["remove"] = &Player::remove;
	}

	{
		auto meta = lua->new_usertype<Human>("new", sol::no_constructor);
		meta["vehicleSeat"] = &Human::vehicleSeat;
		meta["despawnTime"] = &Human::despawnTime;
		meta["damage"] = &Human::damage;
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

		meta["index"] = sol::property(&Human::getIndex);
		meta["isActive"] = sol::property(&Human::getIsActive, &Human::setIsActive);
		meta["isAlive"] = sol::property(&Human::getIsAlive, &Human::setIsAlive);
		meta["isImmortal"] = sol::property(&Human::getIsImmortal, &Human::setIsImmortal);
		meta["isOnGround"] = sol::property(&Human::getIsOnGround);
		meta["isStanding"] = sol::property(&Human::getIsStanding);
		meta["isBleeding"] = sol::property(&Human::getIsBleeding, &Human::setIsBleeding);
		meta["player"] = sol::property(&Human::getPlayer);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);

		meta["remove"] = &Human::remove;
		meta["getPos"] = &Human::getPos;
		meta["setPos"] = &Human::setPos;
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

		meta["index"] = sol::property(&ItemType::getIndex);
		meta["name"] = sol::property(&ItemType::getName, &ItemType::setName);
		meta["isGun"] = sol::property(&ItemType::getIsGun, &ItemType::setIsGun);
	}

	{
		auto meta = lua->new_usertype<Item>("new", sol::no_constructor);
		meta["type"] = &Item::type;
		meta["parentSlot"] = &Item::parentSlot;
		meta["pos"] = &Item::pos;
		meta["vel"] = &Item::vel;
		meta["rot"] = &Item::rot;
		meta["bullets"] = &Item::bullets;

		meta["index"] = sol::property(&Item::getIndex);
		meta["isActive"] = sol::property(&Item::getIsActive, &Item::setIsActive);
		meta["hasPhysics"] = sol::property(&Item::getHasPhysics, &Item::setHasPhysics);
		meta["physicsSettled"] = sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled);
		meta["rigidBody"] = sol::property(&Item::getRigidBody);
		meta["parentHuman"] = sol::property(&Item::getParentHuman);
		meta["parentItem"] = sol::property(&Item::getParentItem);

		meta["update"] = &Item::update;
		meta["remove"] = &Item::remove;
		meta["mountItem"] = &Item::mountItem;
		meta["speak"] = &Item::speak;
		meta["explode"] = &Item::explode;
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
		meta["gearX"] = &Vehicle::gearX;
		meta["steerControl"] = &Vehicle::steerControl;
		meta["gearY"] = &Vehicle::gearY;
		meta["gasControl"] = &Vehicle::gasControl;
		meta["bladeBodyID"] = &Vehicle::bladeBodyID;

		meta["index"] = sol::property(&Vehicle::getIndex);
		meta["isActive"] = sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive);
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

		meta["player"] = sol::property(&Bullet::getPlayer);
	}

	{
		auto meta = lua->new_usertype<Bone>("new", sol::no_constructor);
		meta["pos"] = &Bone::pos;
		meta["pos2"] = &Bone::pos2;
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

		meta["index"] = sol::property(&RigidBody::getIndex);
		meta["isActive"] = sol::property(&RigidBody::getIsActive, &RigidBody::setIsActive);
		meta["isSettled"] = sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled);
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
	(*lua)["items"]["createRope"] = lua_items_createRope;
	{
		sol::table _meta = lua->create_table();
		(*lua)["items"][sol::metatable_key] = _meta;
		_meta["__index"] = l_items_getByIndex;
	}

	(*lua)["vehicles"] = lua->create_table();
	(*lua)["vehicles"]["getCount"] = l_vehicles_getCount;
	(*lua)["vehicles"]["getAll"] = l_vehicles_getAll;
	(*lua)["vehicles"]["create"] = sol::overload(l_vehicles_create, l_vehicles_createVel);
	(*lua)["vehicles"]["createTraffic"] = l_vehicles_createTraffic;
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

	(*lua)["os"]["setClipboard"] = l_os_setClipboard;
	(*lua)["os"]["listDirectory"] = l_os_listDirectory;

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

		version = (Version*)(exeBase + 0x971B8);

		serverName = (char*)(exeBase + 0x8BBB254);
		serverPort = (unsigned int*)(exeBase + 0x8BBB660);
		isPassworded = (BOOL*)(exeBase + 0x8BBB664);
		password = (char*)(exeBase + 0x8F79ECC);
		gameType = (int*)(exeBase + 0x8F7A248);
		mapName = (char*)(exeBase + 0x8F7A24C);
		loadedMapName = (char*)(exeBase + 0x1339BB24);
		gameState = (int*)(exeBase + 0x8F7A464);
		gameTimer = (int*)(exeBase + 0x8F7A46C);
		isLevelLoaded = (BOOL*)(exeBase + 0x1339BB20);

		lineIntersectResult = (RayCastResult*)(exeBase + 0x1A0515E0);
		sunTime = (unsigned int*)(exeBase + 0x351EAEA0);

		connections = (Connection*)(exeBase + 0x1ED22060);
		accounts = (Account*)(exeBase + 0x17B0DAF0);
		players = (Player*)(exeBase + 0x13AFCE60);
		humans = (Human*)(exeBase + 0x5CC3C8);
		vehicles = (Vehicle*)(exeBase + 0x81B23E0);
		itemTypes = (ItemType*)(exeBase + 0x1A0639C0);
		items = (Item*)(exeBase + 0x90331E0);
		bullets = (Bullet*)(exeBase + 0x24F860);
		bodies = (RigidBody*)(exeBase + 0x9B940);

		numConnections = (unsigned int*)(exeBase + 0x9D33E0);
		numBullets = (unsigned int*)(exeBase + 0x27DE2C20);

		playerai = (playerai_func)(exeBase + 0x79D50);
		rigidbodysimulation = (void_func)(exeBase + 0x11060);
		objectsimulation = (void_func)(exeBase + 0x82040);
		itemsimulation = (void_func)(exeBase + 0x7F760);
		humansimulation = (void_func)(exeBase + 0x8E0A0);

		logicsimulation = (void_func)(exeBase + 0x91E10);
		logicsimulation_race = (void_func)(exeBase + 0x89BE0);
		logicsimulation_round = (void_func)(exeBase + 0x8C9C0);
		logicsimulation_world = (void_func)(exeBase + 0x8D8C0);
		logicsimulation_terminator = (void_func)(exeBase + 0x8A380);
		logicsimulation_coop = (void_func)(exeBase + 0x898A0);
		logicsimulation_versus = (void_func)(exeBase + 0x8B820);

		recvpacket = (recvpacket_func)(exeBase + 0x781E0);
		sendpacket = (void_func)(exeBase + 0x77530);
		bulletsimulation = (void_func)(exeBase + 0x64620);
		bullettimetolive = (void_func)(exeBase + 0x1C9E0);

		resetgame = (void_func)(exeBase + 0x85180);
		scenario_createtraffic3 = (void_index_func)(exeBase + 0x773D0);

		scenario_armhuman = (armhuman_func)(exeBase + 0x544A0);

		linkitem = (grabitem_func)(exeBase + 0x4A5B0);
		chat = (chat_func)(exeBase + 0x65420);

		createlevel = (void_func)(exeBase + 0x7FDE0);

		createplayer = (createplayer_func)(exeBase + 0x34530);
		createhuman = (createhuman_func)(exeBase + 0x7CE40);
		createitem = (createitem_func)(exeBase + 0x49CE0);
		createrope = (createrope_func)(exeBase + 0x4AF60);
		createvehicle = (createvehicle_func)(exeBase + 0x4E940);

		playerdeathtax = (void_index_func)(exeBase + 0x1EB80);
		human_applydamage = (human_applydamage_func)(exeBase + 0x9130);

		deleteplayer = (void_index_func)(exeBase + 0xFCB0);
		deletehuman = (void_index_func)(exeBase + 0x416C0);
		deleteitem = (void_index_func)(exeBase + 0x4A080);
		deleteobject = (void_index_func)(exeBase + 0x2C340);
		grenadeexplosion = (void_index_func)(exeBase + 0x251E0);

		createevent_message = (createevent_message_func)(exeBase + 0x7700);
		createevent_updateplayer = (void_index_func)(exeBase + 0x78C0);
		createevent_updateplayer_finance = (void_index_func)(exeBase + 0x79E0);
		createevent_updateitem = (void_index_func)(exeBase + 0x7820);
		createevent_createobject = (void_index_func)(exeBase + 0x77B0);
		createevent_updateobject = (createevent_updateobject_func)(exeBase + 0x1EEC0);
		createevent_sound = (createevent_sound_func)(exeBase + 0x1EF90);
		createevent_explosion = (createevent_explosion_func)(exeBase + 0x1F0F0);
		createevent_updatedoor = (createevent_updatedoor_func)(exeBase + 0x7AD0);
		createevent_bullethit = (createevent_bullethit_func)(exeBase + 0x1EE20);

		lineintersecthuman = (lineintersecthuman_func)(exeBase + 0x41890);
		lineintersectlevel = (lineintersectlevel_func)(exeBase + 0x4B440);
		lineintersectobject = (lineintersectobject_func)(exeBase + 0x2EE00);
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