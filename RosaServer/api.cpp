#include "pch.h"

#include "api.h"

void printLuaError(sol::error* err) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("Lua error:\n%s\n\n", err->what());
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

bool noLuaCallError(sol::protected_function_result* res) {
	if (res->valid()) return true;
	sol::error err = *res;
	printLuaError(&err);
	return false;
}

bool noLuaCallError(sol::load_result* res) {
	if (res->valid()) return true;
	sol::error err = *res;
	printLuaError(&err);
	return false;
}

void hookAndReset(int reason) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ResetGame", reason);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		resetgame();
		if (func != sol::nil) {
			auto res = func("PostResetGame", reason);
			noLuaCallError(&res);
		}
	}
}

class tcon : public tinyConsole
{
public:
	tcon() : tinyConsole() { ; }

	int trigger(std::string s)
	{
		consoleQueue.push(s);
		return 0;
	}
};

DWORD WINAPI ConsoleThread(HMODULE hModule) {
	tcon console;
	console.run();

	return 0;
}

void l_printAppend(const char* str) {
	printf("%s", str);
}

void l_flagStateForReset(const char* mode) {
	hookMode = mode;
	shouldReset = true;
}

Vector l_Vector() {
	return Vector{ 0.f, 0.f, 0.f };
}

Vector l_Vector_3f(float x, float y, float z) {
	return Vector{ x, y, z };
}

RotMatrix l_RotMatrix(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
	return RotMatrix{ x1, y1, z1, x2, y2, z2, x3, y3, z3 };
}

void l_http_post(const char* host, int port, const char* path, sol::table headers, const char* body, const char* contentType) {
	LuaHTTPRequest request {
		LuaRequestType::post,
		host,
		(unsigned short)port,
		path,
		contentType,
		body
	};

	for (const auto& pair : headers)
		request.headers.emplace(pair.first.as<std::string>(), pair.second.as<std::string>());

	requestQueue.push(request);
}

DWORD WINAPI HTTPThread(HMODULE hModule) {
	while (true) {
		while (!requestQueue.empty()) {
			auto req = requestQueue.front();

			switch (req.type) {
				case post:
					httplib::Client client(req.host.c_str(), req.port);

					auto res = client.Post(req.path.c_str(), req.headers, req.body.c_str(), req.contentType.c_str());
					/*if (res) {
						std::cout << res->status << std::endl;
						std::cout << res->get_header_value("Content-Type") << std::endl;
						std::cout << res->body << std::endl;
					}*/
					break;
			}

			requestQueue.pop();
		}
		Sleep(16);
	}

	return 0;
}

void l_event_sound(int soundType, Vector* pos, float volume, float pitch) {
	createevent_sound(soundType, pos, volume, pitch);
}

void l_event_soundSimple(int soundType, Vector* pos) {
	createevent_sound(soundType, pos, 1.0f, 1.0f);
}

void l_event_explosion(Vector* pos) {
	createevent_explosion(0, pos);
}

void l_event_bulletHit(int hitType, Vector* pos, Vector* normal) {
	createevent_bullethit(0, hitType, pos, normal);
}

sol::table l_physics_lineIntersectLevel(Vector* posA, Vector* posB) {
	sol::table table = lua->create_table();
	BOOL res = lineintersectlevel(posA, posB);
	if (res) {
		table["pos"] = lineIntersectResult->pos;
		table["normal"] = lineIntersectResult->normal;
		table["fraction"] = lineIntersectResult->fraction;
	}
	table["hit"] = res != 0;
	return table;
}

sol::table l_physics_lineIntersectHuman(Human* man, Vector* posA, Vector* posB) {
	sol::table table = lua->create_table();
	BOOL res = lineintersecthuman(man->getIndex(), posA, posB);
	if (res) {
		table["pos"] = lineIntersectResult->pos;
		table["normal"] = lineIntersectResult->normal;
		table["fraction"] = lineIntersectResult->fraction;
		table["bone"] = lineIntersectResult->humanBone;
	}
	table["hit"] = res != 0;
	return table;
}

sol::table l_physics_lineIntersectVehicle(Vehicle* vcl, Vector* posA, Vector* posB) {
	sol::table table = lua->create_table();
	BOOL res = lineintersectobject(vcl->getIndex(), posA, posB);
	if (res) {
		table["pos"] = lineIntersectResult->pos;
		table["normal"] = lineIntersectResult->normal;
		table["fraction"] = lineIntersectResult->fraction;
		table["bone"] = lineIntersectResult->humanBone;
	}
	table["hit"] = res != 0;
	return table;
}

void l_physics_garbageCollectBullets() {
	bullettimetolive();
}

sol::table l_itemTypes_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFITEMTYPES; i++) {
		arr.add(&itemTypes[i]);
	}
	return arr;
}

ItemType* l_itemTypes_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFITEMTYPES)
		throw std::runtime_error("Index out of range");
	return &itemTypes[idx];
}

int l_items_getCount() {
	int count = 0;
	for (int i = 0; i < MAXNUMOFITEMS; i++) {
		if ((&items[i])->active) count++;
	}
	return count;
}

sol::table l_items_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFITEMS; i++) {
		auto item = &items[i];
		if (!item->active) continue;
		arr.add(item);
	}
	return arr;
}

Item* l_items_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFITEMS)
		throw std::runtime_error("Index out of range");
	return &items[idx];
}

Item* l_items_create(int itemType, Vector* pos, RotMatrix* rot) {
	int id = createitem(itemType, pos, nullptr, rot);
	return id == -1 ? nullptr : &items[id];
}

Item* l_items_createVel(int itemType, Vector* pos, Vector* vel, RotMatrix* rot) {
	int id = createitem(itemType, pos, vel, rot);
	return id == -1 ? nullptr : &items[id];
}

void lua_items_createRope(Vector* pos, RotMatrix* rot) {
	int x = createrope(pos, rot);
	printf("createrope => %i\n", x);
}

sol::table l_vehicles_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFVEHICLES; i++) {
		auto vcl = &vehicles[i];
		if (!vcl->active) continue;
		arr.add(vcl);
	}
	return arr;
}

Vehicle* l_vehicles_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFVEHICLES)
		throw std::runtime_error("Index out of range");
	return &vehicles[idx];
}

Vehicle* l_vehicles_create(int type, Vector* pos, RotMatrix* rot, int color) {
	int id = createvehicle(type, pos, nullptr, rot, color);
	return id == -1 ? nullptr : &vehicles[id];
}

Vehicle* l_vehicles_createVel(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color) {
	int id = createvehicle(type, pos, vel, rot, color);
	return id == -1 ? nullptr : &vehicles[id];
}

void l_vehicles_createTraffic(int density) {
	scenario_createtraffic3(density);
}

void l_chat_announce(const char* message) {
	createevent_message(0, (char*)message, -1, 0);
}

void l_chat_tellAdmins(const char* message) {
	createevent_message(4, (char*)message, -1, 0);
}

void l_chat_addRaw(int type, const char* message, int speakerID, int distance) {
	createevent_message(type, (char*)message, speakerID, distance);
}

sol::table l_accounts_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; ; i++) {
		Account* acc = &accounts[i];
		if (!acc->subRosaID) break;
		arr.add(acc);
	}
	return arr;
}

Account* l_accounts_getByPhone(int phone) {
	for (int i = 0; ; i++) {
		Account* acc = &accounts[i];
		if (!acc->subRosaID) break;
		if (acc->phoneNumber == phone)
			return acc;
	}
	return nullptr;
}

sol::table l_players_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFPLAYERS; i++) {
		auto ply = &players[i];
		if (!ply->active) continue;
		arr.add(ply);
	}
	return arr;
}

Player* l_players_getByPhone(int phone) {
	for (int i = 0; i < MAXNUMOFPLAYERS; i++) {
		auto ply = &players[i];
		if (!ply->active) continue;
		if (ply->phoneNumber == phone)
			return ply;
	}
	return nullptr;
}

sol::table l_players_getNonBots() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFPLAYERS; i++) {
		auto ply = &players[i];
		if (!ply->active || !ply->subRosaID || ply->isBot) continue;
		arr.add(ply);
	}
	return arr;
}

Player* l_players_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFPLAYERS)
		throw std::runtime_error("Index out of range");
	return &players[idx];
}

Player* l_players_createBot() {
	int playerID = createplayer();
	if (playerID == -1) return nullptr;
	auto ply = &players[playerID];
	ply->subRosaID = 0;
	ply->isBot = TRUE;
	ply->team = 6;
	ply->setName("Bot");
	return ply;
}

sol::table l_humans_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFHUMANS; i++) {
		auto man = &humans[i];
		if (!man->active) continue;
		arr.add(man);
	}
	return arr;
}

Human* l_humans_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFHUMANS)
		throw std::runtime_error("Index out of range");
	return &humans[idx];
}

Human* l_humans_create(Vector* pos, RotMatrix* rot, Player* ply) {
	int playerID = ply->getIndex();
	if (ply->humanID != -1)
		deletehuman(ply->humanID);
	int humanID = createhuman(pos, rot, playerID);
	if (humanID == -1)
		return nullptr;
	auto man = &humans[humanID];
	man->playerID = playerID;
	ply->humanID = humanID;
	return man;
}

sol::table l_bullets_getAll() {
	auto arr = lua->create_table();
	for (unsigned int i = 0; i < *numBullets; i++) {
		Bullet* bul = &bullets[i];
		arr.add(bul);
	}
	return arr;
}

int l_rigidBodies_getCount() {
	int count = 0;
	for (int i = 0; i < MAXNUMOFRIGIDBODIES; i++) {
		if ((&bodies[i])->active) count++;
	}
	return count;
}

sol::table l_rigidBodies_getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < MAXNUMOFRIGIDBODIES; i++) {
		auto body = &bodies[i];
		if (!body->active) continue;
		arr.add(body);
	}
	return arr;
}

RigidBody* l_rigidBodies_getByIndex(unsigned int idx) {
	if (idx >= MAXNUMOFRIGIDBODIES)
		throw std::runtime_error("Index out of range");
	return &bodies[idx];
}

//dumbass false positive
#pragma warning( push )
#pragma warning( disable : 6387 )
void l_os_setClipboard(std::string s) {
	if (s.length() > 0 && OpenClipboard(0)) {
		EmptyClipboard();
		HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
		if (hg != NULL) {
			memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
			GlobalUnlock(hg);
			SetClipboardData(CF_TEXT, hg);
			CloseClipboard();
			GlobalFree(hg);
		}
		else {
			CloseClipboard();
			return;
		}
	}
}
#pragma warning( pop ) 

sol::table l_os_listDirectory(const char* path) {
	auto arr = lua->create_table();
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		auto table = lua->create_table();
		table["isDirectory"] = entry.is_directory();
		table["name"] = entry.path().filename().string();
		table["stem"] = entry.path().stem().string();
		table["extension"] = entry.path().extension().string();
		arr.add(table);
	}
	return arr;
}

std::string Connection::getAddress() {
	char buf[16];
	sprintf(buf, "%i.%i.%i.%i",
		(int)address[3],
		(int)address[2],
		(int)address[1],
		(int)address[0]
	);
	return buf;
}

int Account::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)accounts) / sizeof(*this);
}

void Vector::add(Vector* other) {
	x += other->x;
	y += other->y;
	z += other->z;
}

void Vector::mult(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

void Vector::set(Vector* other) {
	x = other->x;
	y = other->y;
	z = other->z;
}

Vector Vector::clone() const {
	return Vector{ x, y, z };
}

float Vector::dist(Vector* other) const {
	float dx = x - other->x;
	float dy = y - other->y;
	float dz = z - other->z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

float Vector::distSquare(Vector* other) const {
	float dx = x - other->x;
	float dy = y - other->y;
	float dz = z - other->z;
	return dx * dx + dy * dy + dz * dz;
}

void RotMatrix::set(RotMatrix* other) {
	x1 = other->x1;
	y1 = other->y1;
	z1 = other->z1;

	x2 = other->x2;
	y2 = other->y2;
	z2 = other->z2;

	x3 = other->x3;
	y3 = other->y3;
	z3 = other->z3;
}

RotMatrix RotMatrix::clone() const {
	return RotMatrix{x1, y1, z1, x2, y2, z2, x3, y3, z3};
}

int Player::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)players) / sizeof(*this);
}

void Player::update() const {
	createevent_updateplayer(getIndex());
}

void Player::updateFinance() const {
	createevent_updateplayer_finance(getIndex());
}

void Player::remove() const {
	deleteplayer(getIndex());
}

Human* Player::getHuman() {
	if (humanID == -1)
		return nullptr;
	return &humans[humanID];
}

Connection* Player::getConnection() {
	int id = getIndex();
	for (unsigned int i = 0; i < *numConnections; i++) {
		auto con = &connections[i];
		if (con->playerID == id)
			return con;
	}
	return nullptr;
}

Account* Player::getAccount() {
	return &accounts[accountID];
}

void Player::setAccount(Account* account) {
	if (account == nullptr)
		throw std::runtime_error("Cannot set account to nil value");
	else
		accountID = account->getIndex();
}

int Human::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)humans) / sizeof(*this);
}

void Human::remove() const {
	deletehuman(getIndex());
}

Player* Human::getPlayer() const {
	if (playerID == -1)
		return nullptr;
	return &players[playerID];
}

Vehicle* Human::getVehicle() const {
	if (vehicleID == -1)
		return nullptr;
	return &vehicles[vehicleID];
}

void Human::setVehicle(Vehicle* vcl) {
	if (vcl == nullptr)
		vehicleID = -1;
	else
		vehicleID = vcl->getIndex();
}

Vector Human::getPos() const {
	return pos;
};

void Human::setPos(Vector* vec) {
	float offX = vec->x - pos.x;
	float offY = vec->y - pos.y;
	float offZ = vec->z - pos.z;

	Bone* bone;
	RigidBody* body;
	for (int i = 0; i < 16; i++) {
		bone = &bones[i];
		bone->pos.x += offX;
		bone->pos.y += offY;
		bone->pos.z += offZ;
		bone->pos2.x += offX;
		bone->pos2.y += offY;
		bone->pos2.z += offZ;

		body = &bodies[bone->bodyID];
		body->pos.x += offX;
		body->pos.y += offY;
		body->pos.z += offZ;
	}
};

void Human::speak(const char* message, int distance) const {
	createevent_message(1, (char*)message, getIndex(), distance);
}

void Human::arm(int weapon, int magCount) const {
	scenario_armhuman(getIndex(), weapon, magCount);
}

Bone* Human::getBone(unsigned int idx) {
	if (idx > 15)
		throw std::runtime_error("Index out of range");

	return &bones[idx];
}

RigidBody* Human::getRigidBody(unsigned int idx) const {
	if (idx > 15)
		throw std::runtime_error("Index out of range");

	return &bodies[bones[idx].bodyID];
}

void Human::setVelocity(Vector* vel) const {
	for (int i = 0; i < 16; i++) {
		auto body = getRigidBody(i);
		body->vel.set(vel);
	}
}

void Human::addVelocity(Vector* vel) const {
	for (int i = 0; i < 16; i++) {
		auto body = getRigidBody(i);
		body->vel.add(vel);
	}
}

bool Human::mountItem(Item* childItem, unsigned int slot) const {
	return linkitem(childItem->getIndex(), -1, getIndex(), slot);
}

void Human::applyDamage(int bone, int damage) const {
	human_applydamage(getIndex(), bone, 0, damage);
}

int ItemType::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)itemTypes) / sizeof(*this);
}

int Item::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)items) / sizeof(*this);
}

void Item::update() const {
	createevent_updateitem(getIndex());
}

void Item::remove() const {
	deleteitem(getIndex());
}

Human* Item::getParentHuman() const {
	return parentHumanID == -1 ? nullptr : &humans[parentHumanID];
}

Item* Item::getParentItem() const {
	return parentItemID == -1 ? nullptr : &items[parentItemID];
}

RigidBody* Item::getRigidBody() const {
	return &bodies[bodyID];
}

bool Item::mountItem(Item* childItem, unsigned int slot) const {
	return linkitem(getIndex(), childItem->getIndex(), -1, slot);
}

void Item::speak(const char* message, int distance) const {
	createevent_message(2, (char*)message, getIndex(), distance);
}

void Item::explode() const {
	grenadeexplosion(getIndex());
}

int Vehicle::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)vehicles) / sizeof(*this);
}

void Vehicle::updateType() const {
	createevent_createobject(getIndex());
}

void Vehicle::updateDestruction(int updateType, int partID, Vector* pos, Vector* normal) const {
	createevent_updateobject(getIndex(), updateType, partID, pos, normal);
}

void Vehicle::remove() const {
	deleteobject(getIndex());
}

Player* Vehicle::getLastDriver() const {
	if (lastDriverPlayerID == -1)
		return nullptr;
	return &players[lastDriverPlayerID];
}

RigidBody* Vehicle::getRigidBody() const {
	return &bodies[bodyID];
}

Player* Bullet::getPlayer() const {
	if (playerID == -1)
		return nullptr;
	return &players[playerID];
}


int RigidBody::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)bodies) / sizeof(*this);
}