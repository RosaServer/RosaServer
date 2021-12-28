#include "api.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <limits>

#include "console.h"

bool initialized = false;
bool shouldReset = false;

sol::state* lua;
std::string hookMode;

sol::table* accountDataTables[maxNumberOfAccounts] = {0};
sol::table* playerDataTables[maxNumberOfPlayers] = {0};
sol::table* humanDataTables[maxNumberOfHumans] = {0};
sol::table* itemDataTables[maxNumberOfItems] = {0};
sol::table* vehicleDataTables[maxNumberOfVehicles] = {0};
sol::table* bodyDataTables[maxNumberOfRigidBodies] = {0};

std::mutex stateResetMutex;

static constexpr const char* errorOutOfRange = "Index out of range";
static constexpr const char* missingArgument = "Missing argument";

void printLuaError(sol::error* err) {
	std::ostringstream stream;

	stream << "\033[41;1m Lua error \033[0m\n\033[31m";
	stream << err->what();
	stream << "\033[0m\n";

	Console::log(stream.str());
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
	if (Hooks::enabledKeys[Hooks::EnableKeys::ResetGame]) {
		bool noParent = false;
		if (Hooks::run != sol::nil) {
			auto res = Hooks::run("ResetGame", reason);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&Hooks::resetGameHook);
				Engine::resetGame();
			}
			if (Hooks::run != sol::nil) {
				auto res = Hooks::run("PostResetGame", reason);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&Hooks::resetGameHook);
		Engine::resetGame();
	}
}

namespace Lua {
void print(sol::variadic_args args, sol::this_state s) {
	sol::state_view lua(s);

	sol::protected_function toString = lua["tostring"];
	if (toString == sol::nil) {
		return;
	}

	std::ostringstream stream;

	bool doneFirst = false;
	for (auto arg : args) {
		if (doneFirst)
			stream << '\t';
		else
			doneFirst = true;

		auto stringified = toString(arg);

		if (!noLuaCallError(&stringified)) {
			return;
		}

		std::string str = stringified;
		stream << str;
	}

	stream << '\n';

	Console::log(stream.str());
}

void flagStateForReset(const char* mode) {
	hookMode = mode;
	shouldReset = true;
}

Vector Vector_() { return Vector{0.f, 0.f, 0.f}; }

Vector Vector_3f(float x, float y, float z) { return Vector{x, y, z}; }

RotMatrix RotMatrix_(float x1, float y1, float z1, float x2, float y2, float z2,
                     float x3, float y3, float z3) {
	return RotMatrix{x1, y1, z1, x2, y2, z2, x3, y3, z3};
}

static sol::object handleSyncHTTPResponse(httplib::Result& res,
                                          sol::this_state s) {
	sol::state_view lua(s);

	if (res) {
		sol::table table = lua.create_table();
		table["status"] = res->status;
		table["body"] = res->body;

		sol::table headers = lua.create_table();
		for (const auto& h : res->headers) headers[h.first] = h.second;
		table["headers"] = headers;

		return sol::make_object(lua, table);
	}

	return sol::make_object(lua, sol::nil);
}

sol::object http::getSync(const char* scheme, const char* path,
                          sol::table headers, sol::this_state s) {
	httplib::Client client(scheme);
	client.set_connection_timeout(6);
	client.set_keep_alive(false);

	httplib::Headers httpHeaders;
	for (const auto& pair : headers)
		httpHeaders.emplace(pair.first.as<std::string>(),
		                    pair.second.as<std::string>());

	httpHeaders.emplace("Connection", "close");

	auto res = client.Get(path, httpHeaders);
	return handleSyncHTTPResponse(res, s);
}

sol::object http::postSync(const char* scheme, const char* path,
                           sol::table headers, std::string body,
                           const char* contentType, sol::this_state s) {
	httplib::Client client(scheme);
	client.set_connection_timeout(6);
	client.set_keep_alive(false);

	httplib::Headers httpHeaders;
	for (const auto& pair : headers)
		httpHeaders.emplace(pair.first.as<std::string>(),
		                    pair.second.as<std::string>());

	httpHeaders.emplace("Connection", "close");

	auto res = client.Post(path, httpHeaders, body, contentType);
	return handleSyncHTTPResponse(res, s);
}

static inline std::string withoutPostPrefix(std::string name) {
	if (name.rfind("Post", 0) == 0) {
		return name.substr(4);
	}

	return name;
}

bool hook::enable(std::string name) {
	auto search = Hooks::enableNames.find(withoutPostPrefix(name));
	if (search != Hooks::enableNames.end()) {
		Hooks::enabledKeys[search->second] = true;
		return true;
	}
	return false;
}

bool hook::disable(std::string name) {
	auto search = Hooks::enableNames.find(withoutPostPrefix(name));
	if (search != Hooks::enableNames.end()) {
		Hooks::enabledKeys[search->second] = false;
		return true;
	}
	return false;
}

void hook::clear() {
	for (size_t i = 0; i < Hooks::EnableKeys::SIZE; i++) {
		Hooks::enabledKeys[i] = false;
	}
}

sol::table physics::lineIntersectLevel(Vector* posA, Vector* posB,
                                       bool onlyCity) {
	sol::table table = lua->create_table();
	subhook::ScopedHookRemove remove(&Hooks::lineIntersectLevelHook);
	int res = Engine::lineIntersectLevel(posA, posB, !onlyCity);
	if (res && (!onlyCity || Engine::lineIntersectResult->areaId != -1)) {
		table["pos"] = Engine::lineIntersectResult->pos;
		table["normal"] = Engine::lineIntersectResult->normal;
		table["fraction"] = Engine::lineIntersectResult->fraction;
	}
	table["hit"] = res != 0;
	return table;
}

sol::table physics::lineIntersectHuman(Human* man, Vector* posA, Vector* posB,
                                       float padding) {
	sol::table table = lua->create_table();
	subhook::ScopedHookRemove remove(&Hooks::lineIntersectHumanHook);
	int res = Engine::lineIntersectHuman(man->getIndex(), posA, posB, padding);
	if (res) {
		table["pos"] = Engine::lineIntersectResult->pos;
		table["normal"] = Engine::lineIntersectResult->normal;
		table["fraction"] = Engine::lineIntersectResult->fraction;
		table["bone"] = Engine::lineIntersectResult->humanBone;
	}
	table["hit"] = res != 0;
	return table;
}

sol::table physics::lineIntersectVehicle(Vehicle* vehicle, Vector* posA,
                                         Vector* posB, bool includeWheels) {
	sol::table table = lua->create_table();
	int res = Engine::lineIntersectVehicle(vehicle->getIndex(), posA, posB,
	                                       includeWheels);
	if (res) {
		table["pos"] = Engine::lineIntersectResult->pos;
		table["normal"] = Engine::lineIntersectResult->normal;
		table["fraction"] = Engine::lineIntersectResult->fraction;

		if (Engine::lineIntersectResult->vehicleFace != -1)
			table["face"] = Engine::lineIntersectResult->vehicleFace;
		else
			table["wheel"] = Engine::lineIntersectResult->humanBone;
	}
	table["hit"] = res != 0;
	return table;
}

sol::object physics::lineIntersectLevelQuick(Vector* posA, Vector* posB,
                                             bool onlyCity, sol::this_state s) {
	sol::state_view lua(s);

	subhook::ScopedHookRemove remove(&Hooks::lineIntersectLevelHook);
	int res = Engine::lineIntersectLevel(posA, posB, !onlyCity);
	if (res && (!onlyCity || Engine::lineIntersectResult->areaId != -1)) {
		return sol::make_object(lua, Engine::lineIntersectResult->fraction);
	}
	return sol::make_object(lua, sol::nil);
}

sol::object physics::lineIntersectHumanQuick(Human* man, Vector* posA,
                                             Vector* posB, float padding,
                                             sol::this_state s) {
	sol::state_view lua(s);

	subhook::ScopedHookRemove remove(&Hooks::lineIntersectHumanHook);
	int res = Engine::lineIntersectHuman(man->getIndex(), posA, posB, padding);
	if (res) {
		return sol::make_object(lua, Engine::lineIntersectResult->fraction);
	}
	return sol::make_object(lua, sol::nil);
}

sol::object physics::lineIntersectVehicleQuick(Vehicle* vehicle, Vector* posA,
                                               Vector* posB, bool includeWheels,
                                               sol::this_state s) {
	sol::state_view lua(s);

	int res = Engine::lineIntersectVehicle(vehicle->getIndex(), posA, posB,
	                                       includeWheels);
	if (res) {
		return sol::make_object(lua, Engine::lineIntersectResult->fraction);
	}
	return sol::make_object(lua, sol::nil);
}

std::tuple<sol::object, sol::object> physics::lineIntersectAnyQuick(
    Vector* posA, Vector* posB, Human* ignoreHuman, float humanPadding,
    bool includeWheels, sol::this_state s) {
	sol::state_view lua(s);

	float nearestFraction = std::numeric_limits<float>::infinity();
	void* nearestObject = nullptr;
	bool nearestIsVehicle = false;
	int ignoreHumanId = ignoreHuman ? ignoreHuman->getIndex() : -1;
	bool didHitLevel = false;

	{
		subhook::ScopedHookRemove remove(&Hooks::lineIntersectLevelHook);
		if (Engine::lineIntersectLevel(posA, posB, 1)) {
			nearestFraction = Engine::lineIntersectResult->fraction;
			didHitLevel = true;
		}
	}

	{
		subhook::ScopedHookRemove remove(&Hooks::lineIntersectHumanHook);
		for (int i = 0; i < maxNumberOfHumans; i++) {
			Human* human = &Engine::humans[i];
			if (i != ignoreHumanId && human->active &&
			    Engine::lineIntersectHuman(i, posA, posB, humanPadding)) {
				float fraction = Engine::lineIntersectResult->fraction;
				if (fraction < nearestFraction) {
					nearestFraction = fraction;
					nearestObject = human;
				}
			}
		}
	}

	for (int i = 0; i < maxNumberOfVehicles; i++) {
		Vehicle* vehicle = &Engine::vehicles[i];
		if (vehicle->active &&
		    Engine::lineIntersectVehicle(i, posA, posB, includeWheels)) {
			float fraction = Engine::lineIntersectResult->fraction;
			if (fraction < nearestFraction) {
				nearestFraction = fraction;
				nearestObject = vehicle;
				nearestIsVehicle = true;
			}
		}
	}

	if (nearestObject) {
		if (nearestIsVehicle) {
			return std::make_tuple(
			    sol::make_object(lua, reinterpret_cast<Vehicle*>(nearestObject)),
			    sol::make_object(lua, nearestFraction));
		}
		return std::make_tuple(
		    sol::make_object(lua, reinterpret_cast<Human*>(nearestObject)),
		    sol::make_object(lua, nearestFraction));
	}

	if (didHitLevel) {
		return std::make_tuple(sol::nil, sol::make_object(lua, nearestFraction));
	}

	return std::make_tuple(sol::nil, sol::nil);
}

sol::object physics::lineIntersectTriangle(Vector* outPos, Vector* normal,
                                           Vector* posA, Vector* posB,
                                           Vector* triA, Vector* triB,
                                           Vector* triC, sol::this_state s) {
	sol::state_view lua(s);

	float outFraction;
	int hit = Engine::lineIntersectTriangle(outPos, normal, &outFraction, posA,
	                                        posB, triA, triB, triC);

	if (hit) return sol::make_object(lua, outFraction);
	return sol::make_object(lua, sol::nil);
}

void physics::garbageCollectBullets() { Engine::bulletTimeToLive(); }

void physics::createBlock(int blockX, int blockY, int blockZ,
                          unsigned int flags) {
	short unk[8] = {15, 15, 15, 15, 15, 15, 15, 15};
	subhook::ScopedHookRemove remove(&Hooks::areaCreateBlockHook);
	Engine::areaCreateBlock(0, blockX, blockY, blockZ, flags, unk);
}

void physics::deleteBlock(int blockX, int blockY, int blockZ) {
	subhook::ScopedHookRemove remove(&Hooks::areaDeleteBlockHook);
	Engine::areaDeleteBlock(0, blockX, blockY, blockZ);
}

int itemTypes::getCount() { return maxNumberOfItemTypes; }

sol::table itemTypes::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfItemTypes; i++) {
		arr.add(&Engine::itemTypes[i]);
	}
	return arr;
}

ItemType* itemTypes::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfItemTypes) throw std::invalid_argument(errorOutOfRange);
	return &Engine::itemTypes[idx];
}

ItemType* itemTypes::getByName(const char* name) {
	for (int i = 0; i < maxNumberOfItemTypes; i++) {
		ItemType* type = &Engine::itemTypes[i];
		if (!std::strcmp(name, type->name)) {
			return type;
		}
	}
	return nullptr;
}

int items::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfItems; i++) {
		if ((&Engine::items[i])->active) count++;
	}
	return count;
}

sol::table items::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfItems; i++) {
		auto item = &Engine::items[i];
		if (!item->active) continue;
		arr.add(item);
	}
	return arr;
}

Item* items::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfItems) throw std::invalid_argument(errorOutOfRange);
	return &Engine::items[idx];
}

Item* items::create(ItemType* type, Vector* pos, RotMatrix* rot) {
	return createVel(type, pos, nullptr, rot);
}

Item* items::createVel(ItemType* type, Vector* pos, Vector* vel,
                       RotMatrix* rot) {
	if (type == nullptr) {
		throw std::invalid_argument("Cannot create item with nil type");
	}

	subhook::ScopedHookRemove remove(&Hooks::createItemHook);
	int id = Engine::createItem(type->getIndex(), pos, vel, rot);

	if (id != -1 && itemDataTables[id]) {
		delete itemDataTables[id];
		itemDataTables[id] = nullptr;
	}

	return id == -1 ? nullptr : &Engine::items[id];
}

Item* items::createRope(Vector* pos, RotMatrix* rot) {
	int id = Engine::createRope(pos, rot);
	return id == -1 ? nullptr : &Engine::items[id];
}

int vehicleTypes::getCount() { return maxNumberOfVehicleTypes; }

sol::table vehicleTypes::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfVehicleTypes; i++) {
		arr.add(&Engine::vehicleTypes[i]);
	}
	return arr;
}

VehicleType* vehicleTypes::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfVehicleTypes)
		throw std::invalid_argument(errorOutOfRange);
	return &Engine::vehicleTypes[idx];
}

VehicleType* vehicleTypes::getByName(const char* name) {
	for (int i = 0; i < maxNumberOfVehicleTypes; i++) {
		VehicleType* type = &Engine::vehicleTypes[i];
		if (!std::strcmp(name, type->name)) {
			return type;
		}
	}
	return nullptr;
}

int vehicles::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfVehicles; i++) {
		if ((&Engine::vehicles[i])->active) count++;
	}
	return count;
}

sol::table vehicles::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfVehicles; i++) {
		auto vcl = &Engine::vehicles[i];
		if (!vcl->active) continue;
		arr.add(vcl);
	}
	return arr;
}

Vehicle* vehicles::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfVehicles) throw std::invalid_argument(errorOutOfRange);
	return &Engine::vehicles[idx];
}

Vehicle* vehicles::create(VehicleType* type, Vector* pos, RotMatrix* rot,
                          int color) {
	return createVel(type, pos, nullptr, rot, color);
}

Vehicle* vehicles::createVel(VehicleType* type, Vector* pos, Vector* vel,
                             RotMatrix* rot, int color) {
	if (type == nullptr) {
		throw std::invalid_argument("Cannot create vehicle with nil type");
	}

	subhook::ScopedHookRemove remove(&Hooks::createVehicleHook);
	int id = Engine::createVehicle(type->getIndex(), pos, vel, rot, color);

	if (id != -1 && vehicleDataTables[id]) {
		delete vehicleDataTables[id];
		vehicleDataTables[id] = nullptr;
	}

	return id == -1 ? nullptr : &Engine::vehicles[id];
}

Event* chat::announce(const char* message) {
	return events::createMessage(0, (char*)message, -1, 0);
}

Event* chat::tellAdmins(const char* message) {
	return events::createMessage(4, (char*)message, -1, 0);
}

void accounts::save() {
	subhook::ScopedHookRemove remove(&Hooks::saveAccountsServerHook);
	Engine::saveAccountsServer();
}

int accounts::getCount() {
	int count = 0;
	while (true) {
		Account* acc = &Engine::accounts[count];
		if (!acc->subRosaID) break;
		count++;
	}
	return count;
}

sol::table accounts::getAll() {
	auto arr = lua->create_table();
	for (int i = 0;; i++) {
		Account* acc = &Engine::accounts[i];
		if (!acc->subRosaID) break;
		arr.add(acc);
	}
	return arr;
}

Account* accounts::getByPhone(int phone) {
	for (int i = 0;; i++) {
		Account* acc = &Engine::accounts[i];
		if (!acc->subRosaID) break;
		if (acc->phoneNumber == phone) return acc;
	}
	return nullptr;
}

Account* accounts::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfAccounts) throw std::invalid_argument(errorOutOfRange);
	return &Engine::accounts[idx];
}

int players::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfPlayers; i++) {
		if ((&Engine::players[i])->active) count++;
	}
	return count;
}

sol::table players::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfPlayers; i++) {
		auto ply = &Engine::players[i];
		if (!ply->active) continue;
		arr.add(ply);
	}
	return arr;
}

Player* players::getByPhone(int phone) {
	for (int i = 0; i < maxNumberOfPlayers; i++) {
		auto ply = &Engine::players[i];
		if (!ply->active) continue;
		if (ply->phoneNumber == phone) return ply;
	}
	return nullptr;
}

sol::table players::getNonBots() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfPlayers; i++) {
		auto ply = &Engine::players[i];
		if (!ply->active || !ply->subRosaID || ply->isBot) continue;
		arr.add(ply);
	}
	return arr;
}

sol::table players::getBots() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfPlayers; i++) {
		auto ply = &Engine::players[i];
		if (!ply->active || !ply->isBot) continue;
		arr.add(ply);
	}
	return arr;
}

Player* players::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfPlayers) throw std::invalid_argument(errorOutOfRange);
	return &Engine::players[idx];
}

Player* players::createBot() {
	subhook::ScopedHookRemove remove(&Hooks::createPlayerHook);
	int playerID = Engine::createPlayer();
	if (playerID == -1) return nullptr;

	if (playerDataTables[playerID]) {
		delete playerDataTables[playerID];
		playerDataTables[playerID] = nullptr;
	}

	auto ply = &Engine::players[playerID];
	ply->subRosaID = 0;
	ply->isBot = 1;
	ply->team = 6;
	ply->setName("Bot");
	return ply;
}

int humans::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfHumans; i++) {
		if ((&Engine::humans[i])->active) count++;
	}
	return count;
}

sol::table humans::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfHumans; i++) {
		auto man = &Engine::humans[i];
		if (!man->active) continue;
		arr.add(man);
	}
	return arr;
}

Human* humans::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfHumans) throw std::invalid_argument(errorOutOfRange);
	return &Engine::humans[idx];
}

Human* humans::create(Vector* pos, RotMatrix* rot, Player* ply) {
	int playerID = ply->getIndex();
	if (ply->humanID != -1) {
		subhook::ScopedHookRemove remove(&Hooks::deleteHumanHook);
		Engine::deleteHuman(ply->humanID);
	}
	int humanID;
	{
		subhook::ScopedHookRemove remove(&Hooks::createHumanHook);
		humanID = Engine::createHuman(pos, rot, playerID);
	}
	if (humanID == -1) return nullptr;

	if (humanDataTables[humanID]) {
		delete humanDataTables[humanID];
		humanDataTables[humanID] = nullptr;
	}

	auto man = &Engine::humans[humanID];
	man->playerID = playerID;
	ply->humanID = humanID;
	return man;
}

unsigned int bullets::getCount() { return *Engine::numBullets; }

sol::table bullets::getAll() {
	auto arr = lua->create_table();
	for (unsigned int i = 0; i < *Engine::numBullets; i++) {
		Bullet* bul = &Engine::bullets[i];
		arr.add(bul);
	}
	return arr;
}

Bullet* bullets::create(int type, Vector* pos, Vector* vel, Player* ply) {
	subhook::ScopedHookRemove remove(&Hooks::createBulletHook);
	int bulletID = Engine::createBullet(type, pos, vel,
	                                    ply == nullptr ? -1 : ply->getIndex());
	return bulletID == -1 ? nullptr : &Engine::bullets[bulletID];
}

int rigidBodies::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfRigidBodies; i++) {
		if ((&Engine::bodies[i])->active) count++;
	}
	return count;
}

sol::table rigidBodies::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfRigidBodies; i++) {
		auto body = &Engine::bodies[i];
		if (!body->active) continue;
		arr.add(body);
	}
	return arr;
}

RigidBody* rigidBodies::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfRigidBodies)
		throw std::invalid_argument(errorOutOfRange);
	return &Engine::bodies[idx];
}

int bonds::getCount() {
	int count = 0;
	for (int i = 0; i < maxNumberOfBonds; i++) {
		if ((&Engine::bonds[i])->active) count++;
	}
	return count;
}

sol::table bonds::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < maxNumberOfBonds; i++) {
		auto bond = &Engine::bonds[i];
		if (!bond->active) continue;
		arr.add(bond);
	}
	return arr;
}

Bond* bonds::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= maxNumberOfBonds) throw std::invalid_argument(errorOutOfRange);
	return &Engine::bonds[idx];
}

int streets::getCount() { return *Engine::numStreets; }

sol::table streets::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < *Engine::numStreets; i++) {
		arr.add(&Engine::streets[i]);
	}
	return arr;
}

Street* streets::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= *Engine::numStreets) throw std::invalid_argument(errorOutOfRange);
	return &Engine::streets[idx];
}

int intersections::getCount() { return *Engine::numStreetIntersections; }

sol::table intersections::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < *Engine::numStreetIntersections; i++) {
		arr.add(&Engine::streetIntersections[i]);
	}
	return arr;
}

StreetIntersection* intersections::getByIndex(sol::table self,
                                              unsigned int idx) {
	if (idx >= *Engine::numStreetIntersections)
		throw std::invalid_argument(errorOutOfRange);
	return &Engine::streetIntersections[idx];
}

int trafficCars::getCount() { return *Engine::numTrafficCars; }

sol::table trafficCars::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < *Engine::numTrafficCars; i++) {
		arr.add(&Engine::trafficCars[i]);
	}
	return arr;
}

TrafficCar* trafficCars::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= *Engine::numTrafficCars)
		throw std::invalid_argument(errorOutOfRange);
	return &Engine::trafficCars[idx];
}

void trafficCars::createMany(int amount) {
	subhook::ScopedHookRemove remove(&Hooks::createTrafficHook);
	Engine::createTraffic(amount);
}

int buildings::getCount() { return *Engine::numBuildings; }

sol::table buildings::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < *Engine::numBuildings; i++) {
		arr.add(&Engine::buildings[i]);
	}
	return arr;
}

Building* buildings::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= *Engine::numBuildings)
		throw std::invalid_argument(errorOutOfRange);
	return &Engine::buildings[idx];
}

int events::getCount() { return *Engine::numEvents; }

sol::table events::getAll() {
	auto arr = lua->create_table();
	for (int i = 0; i < *Engine::numEvents; i++) {
		arr.add(&Engine::events[i]);
	}
	return arr;
}

Event* events::getByIndex(sol::table self, unsigned int idx) {
	if (idx >= *Engine::numEvents) throw std::invalid_argument(errorOutOfRange);
	return &Engine::events[idx];
}

Event* events::createBullet(int bulletType, Vector* pos, Vector* vel,
                            Item* item) {
	subhook::ScopedHookRemove remove(&Hooks::createEventBulletHook);
	Engine::createEventBullet(bulletType, pos, vel,
	                          item == nullptr ? -1 : item->getIndex());
	return &Engine::events[*Engine::numEvents - 1];
}

Event* events::createBulletHit(int hitType, Vector* pos, Vector* normal) {
	subhook::ScopedHookRemove remove(&Hooks::createEventBulletHitHook);
	Engine::createEventBulletHit(0, hitType, pos, normal);
	return &Engine::events[*Engine::numEvents - 1];
}

Event* events::createMessage(int messageType, const char* message,
                             int speakerID, int volumeLevel) {
	subhook::ScopedHookRemove remove(&Hooks::createEventMessageHook);
	Engine::createEventMessage(messageType, (char*)message, speakerID,
	                           volumeLevel);
	return &Engine::events[*Engine::numEvents - 1];
}

Event* events::createSound(int soundType, Vector* pos, float volume,
                           float pitch) {
	subhook::ScopedHookRemove remove(&Hooks::createEventSoundHook);
	Engine::createEventSound(soundType, pos, volume, pitch);
	return &Engine::events[*Engine::numEvents - 1];
}

Event* events::createSoundSimple(int soundType, Vector* pos) {
	subhook::ScopedHookRemove remove(&Hooks::createEventSoundHook);
	Engine::createEventSound(soundType, pos, 1.0f, 1.0f);
	return &Engine::events[*Engine::numEvents - 1];
}

Event* events::createExplosion(Vector* pos) {
	Engine::createEventExplosion(0, pos);
	return &Engine::events[*Engine::numEvents - 1];
}

sol::table os::listDirectory(std::string_view path, sol::this_state s) {
	sol::state_view lua(s);

	auto arr = lua.create_table();
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		auto table = lua.create_table();
		auto path = entry.path();
		table["isDirectory"] = std::filesystem::is_directory(path);
		table["name"] = path.filename().string();
		table["stem"] = path.stem().string();
		table["extension"] = path.extension().string();
		arr.add(table);
	}
	return arr;
}

bool os::createDirectory(std::string_view path) {
	return std::filesystem::create_directories(path);
}

double os::getLastWriteTime(std::string_view path) {
	auto lastWriteTime = std::filesystem::last_write_time(path);
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
	                        lastWriteTime.time_since_epoch())
	                        .count();
	return microseconds / 1'000'000.;
}

double os::realClock() {
	auto now = std::chrono::steady_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
	                        now.time_since_epoch())
	                        .count();
	return microseconds / 1'000'000.;
}

void os::exit() { exitCode(EXIT_SUCCESS); }

void os::exitCode(int code) {
	Console::cleanup();
	::exit(code);
}

uintptr_t memory::baseAddress;

uintptr_t memory::getBaseAddress() { return baseAddress; }

uintptr_t memory::getAddressOfConnection(Connection* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfAccount(Account* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfPlayer(Player* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfHuman(Human* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfItemType(ItemType* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfItem(Item* address) { return (uintptr_t)address; }

uintptr_t memory::getAddressOfVehicleType(VehicleType* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfVehicle(Vehicle* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfBullet(Bullet* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfBone(Bone* address) { return (uintptr_t)address; }

uintptr_t memory::getAddressOfRigidBody(RigidBody* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfBond(Bond* address) { return (uintptr_t)address; }

uintptr_t memory::getAddressOfAction(Action* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfMenuButton(MenuButton* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfStreetLane(StreetLane* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfStreet(Street* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfInventorySlot(InventorySlot* address) {
	return (uintptr_t)address;
}

uintptr_t memory::getAddressOfStreetIntersection(StreetIntersection* address) {
	return (uintptr_t)address;
}

int8_t memory::readByte(uintptr_t address) { return *(int8_t*)address; }
uint8_t memory::readUByte(uintptr_t address) { return *(uint8_t*)address; }
int16_t memory::readShort(uintptr_t address) { return *(int16_t*)address; }
uint16_t memory::readUShort(uintptr_t address) { return *(uint16_t*)address; }
int32_t memory::readInt(uintptr_t address) { return *(int32_t*)address; }
uint32_t memory::readUInt(uintptr_t address) { return *(uint32_t*)address; }
int64_t memory::readLong(uintptr_t address) { return *(int64_t*)address; }
uint64_t memory::readULong(uintptr_t address) { return *(uint64_t*)address; }
float memory::readFloat(uintptr_t address) { return *(float*)address; }
double memory::readDouble(uintptr_t address) { return *(double*)address; }

std::string memory::readBytes(uintptr_t address, size_t count) {
	return std::string((char*)address, (char*)(address + count));
}

void memory::writeByte(uintptr_t address, int8_t data) {
	*(int8_t*)address = data;
}
void memory::writeUByte(uintptr_t address, uint8_t data) {
	*(uint8_t*)address = data;
}
void memory::writeShort(uintptr_t address, int16_t data) {
	*(int16_t*)address = data;
}
void memory::writeUShort(uintptr_t address, uint16_t data) {
	*(uint16_t*)address = data;
}
void memory::writeInt(uintptr_t address, int32_t data) {
	*(int32_t*)address = data;
}
void memory::writeUInt(uintptr_t address, uint32_t data) {
	*(uint32_t*)address = data;
}
void memory::writeLong(uintptr_t address, int64_t data) {
	*(int64_t*)address = data;
}
void memory::writeULong(uintptr_t address, uint64_t data) {
	*(uint64_t*)address = data;
}
void memory::writeFloat(uintptr_t address, float data) {
	*(float*)address = data;
}
void memory::writeDouble(uintptr_t address, double data) {
	*(double*)address = data;
}

void memory::writeBytes(uintptr_t address, std::string_view bytes) {
	std::memcpy((void*)address, bytes.data(), bytes.size());
}

};  // namespace Lua

Player* EarShot::getPlayer() const {
	return playerID == -1 ? nullptr : &Engine::players[playerID];
}

void EarShot::setPlayer(Player* player) {
	playerID = player == nullptr ? -1 : player->getIndex();
}

Human* EarShot::getHuman() const {
	return humanID == -1 ? nullptr : &Engine::humans[humanID];
}

void EarShot::setHuman(Human* human) {
	humanID = human == nullptr ? -1 : human->getIndex();
}

Item* EarShot::getReceivingItem() const {
	return receivingItemID == -1 ? nullptr : &Engine::items[receivingItemID];
}

void EarShot::setReceivingItem(Item* item) {
	if (item == nullptr)
		receivingItemID = -1;
	else
		receivingItemID = item->getIndex();
}

Item* EarShot::getTransmittingItem() const {
	return transmittingItemID == -1 ? nullptr
	                                : &Engine::items[transmittingItemID];
}

void EarShot::setTransmittingItem(Item* item) {
	if (item == nullptr)
		transmittingItemID = -1;
	else
		transmittingItemID = item->getIndex();
}

std::string addressFromInteger(unsigned int address) {
	unsigned char* bytes = (unsigned char*)(&address);

	char buf[16];
	sprintf(buf, "%i.%i.%i.%i", (int)bytes[3], (int)bytes[2], (int)bytes[1],
	        (int)bytes[0]);

	return buf;
}

std::string Connection::getAddress() { return addressFromInteger(address); }

Player* Connection::getPlayer() const {
	return playerID == -1 ? nullptr : &Engine::players[playerID];
}

void Connection::setPlayer(Player* player) {
	playerID = player == nullptr ? -1 : player->getIndex();
}

EarShot* Connection::getEarShot(unsigned int idx) {
	if (idx >= 8) throw std::invalid_argument(errorOutOfRange);

	return &earShots[idx];
}

Human* Connection::getSpectatingHuman() const {
	return spectatingHumanID == -1 ? nullptr : &Engine::humans[spectatingHumanID];
}

bool Connection::hasReceivedEvent(Event* event) const {
	if (!event) {
		return false;
	}

	int numEventsUpToThis = event->getIndex() + 1;

	if (*Engine::numEvents < numEventsUpToThis) {
		// The event is no longer valid, the count must have wrapped
		return true;
	}

	return numReceivedEvents >= numEventsUpToThis;
}

std::string Account::__tostring() const {
	char buf[32];
	sprintf(buf, "Account(%i)", getIndex());
	return buf;
}

int Account::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::accounts) / sizeof(*this);
}

sol::table Account::getDataTable() const {
	int index = getIndex();

	if (!accountDataTables[index]) {
		accountDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *accountDataTables[index];
}

std::string Vector::__tostring() const {
	char buf[64];
	sprintf(buf, "Vector(%f, %f, %f)", x, y, z);
	return buf;
}

Vector Vector::__add(Vector* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	return {x + other->x, y + other->y, z + other->z};
}

Vector Vector::__sub(Vector* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	return {x - other->x, y - other->y, z - other->z};
}

Vector Vector::__mul(float scalar) const {
	return {x * scalar, y * scalar, z * scalar};
}

Vector Vector::__mul_RotMatrix(RotMatrix* rot) const {
	if (!rot) throw std::invalid_argument(missingArgument);
	return {rot->x1 * x + rot->y1 * y + rot->z1 * z,
	        rot->x2 * x + rot->y2 * y + rot->z2 * z,
	        rot->x3 * x + rot->y3 * y + rot->z3 * z};
}

Vector Vector::__div(float scalar) const {
	return {x / scalar, y / scalar, z / scalar};
}

Vector Vector::__unm() const { return {-x, -y, -z}; }

void Vector::add(Vector* other) {
	if (!other) throw std::invalid_argument(missingArgument);
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
	if (!other) throw std::invalid_argument(missingArgument);
	x = other->x;
	y = other->y;
	z = other->z;
}

Vector Vector::clone() const { return Vector{x, y, z}; }

double Vector::dist(Vector* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	double dx = x - other->x;
	double dy = y - other->y;
	double dz = z - other->z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

double Vector::distSquare(Vector* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	double dx = x - other->x;
	double dy = y - other->y;
	double dz = z - other->z;
	return dx * dx + dy * dy + dz * dz;
}

double Vector::length() const { return sqrt(x * x + y * y + z * z); }

double Vector::lengthSquare() const { return x * x + y * y + z * z; }

double Vector::dot(Vector* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	return x * other->x + y * other->y + z * other->z;
}

std::tuple<int, int, int> Vector::getBlockPos() const {
	int blockX = x / 4.f;
	int blockY = y / 4.f;
	int blockZ = z / 4.f;
	return std::make_tuple(blockX, blockY, blockZ);
}

void Vector::normalize() {
	double length = this->length();
	x /= length;
	y /= length;
	z /= length;
}

std::string RotMatrix::__tostring() const {
	char buf[256];
	sprintf(buf, "RotMatrix(%f, %f, %f, %f, %f, %f, %f, %f, %f)", x1, y1, z1, x2,
	        y2, z2, x3, y3, z3);
	return buf;
}

RotMatrix RotMatrix::__mul(RotMatrix* other) const {
	if (!other) throw std::invalid_argument(missingArgument);
	return {x1 * other->x1 + y1 * other->x2 + z1 * other->x3,
	        x1 * other->y1 + y1 * other->y2 + z1 * other->y3,
	        x1 * other->z1 + y1 * other->z2 + z1 * other->z3,

	        x2 * other->x1 + y2 * other->x2 + z2 * other->x3,
	        x2 * other->y1 + y2 * other->y2 + z2 * other->y3,
	        x2 * other->z1 + y2 * other->z2 + z2 * other->z3,

	        x3 * other->x1 + y3 * other->x2 + z3 * other->x3,
	        x3 * other->y1 + y3 * other->y2 + z3 * other->y3,
	        x3 * other->z1 + y3 * other->z2 + z3 * other->z3};
}

void RotMatrix::set(RotMatrix* other) {
	if (!other) throw std::invalid_argument(missingArgument);
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

Vector RotMatrix::getForward() const { return Vector{x1, y1, z1}; }

Vector RotMatrix::getUp() const { return Vector{x2, y2, z2}; }

Vector RotMatrix::getRight() const { return Vector{x3, y3, z3}; }

std::string Voice::getFrame(unsigned int idx) const {
	if (idx > 63) throw std::invalid_argument(errorOutOfRange);

	return std::string(reinterpret_cast<const char*>(frames[idx]),
	                   frameSizes[idx]);
}

void Voice::setFrame(unsigned int idx, std::string_view frame,
                     int volumeLevel) {
	if (idx > 63) throw std::invalid_argument(errorOutOfRange);

	frameVolumeLevels[idx] = volumeLevel;
	frameSizes[idx] = frame.size();
	std::memcpy(frames[idx], frame.data(),
	            std::min(std::size_t(2048), frame.size()));
}

std::string Player::__tostring() const {
	char buf[16];
	sprintf(buf, "Player(%i)", getIndex());
	return buf;
}

int Player::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::players) / sizeof(*this);
}

sol::table Player::getDataTable() const {
	int index = getIndex();

	if (!playerDataTables[index]) {
		playerDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *playerDataTables[index];
}

Event* Player::update() const {
	subhook::ScopedHookRemove remove(&Hooks::createEventUpdatePlayerHook);
	Engine::createEventUpdatePlayer(getIndex());
	return &Engine::events[*Engine::numEvents - 1];
}

Event* Player::updateFinance() const {
	Engine::createEventUpdatePlayerFinance(getIndex());
	return &Engine::events[*Engine::numEvents - 1];
}

void Player::remove() const {
	int index = getIndex();

	subhook::ScopedHookRemove remove(&Hooks::deletePlayerHook);
	Engine::deletePlayer(index);

	if (playerDataTables[index]) {
		delete playerDataTables[index];
		playerDataTables[index] = nullptr;
	}
}

void Player::sendMessage(const char* message) const {
	subhook::ScopedHookRemove remove(&Hooks::createEventMessageHook);
	Engine::createEventMessage(6, (char*)message, getIndex(), 0);
}

Human* Player::getHuman() const {
	return humanID == -1 ? nullptr : &Engine::humans[humanID];
}

void Player::setHuman(Human* human) {
	humanID = human == nullptr ? -1 : human->getIndex();
}

Connection* Player::getConnection() {
	int id = getIndex();
	for (unsigned int i = 0; i < *Engine::numConnections; i++) {
		auto con = &Engine::connections[i];
		if (con->playerID == id) return con;
	}
	return nullptr;
}

Account* Player::getAccount() {
	return accountID == -1 ? nullptr : &Engine::accounts[accountID];
}

void Player::setAccount(Account* account) {
	if (account == nullptr)
		throw std::invalid_argument("Cannot set account to nil value");
	else
		accountID = account->getIndex();
}

Voice* Player::getVoice() const { return &Engine::voices[getIndex()]; }

const Vector* Player::getBotDestination() const {
	return !botHasDestination ? nullptr : &botDestination;
}

void Player::setBotDestination(Vector* vec) {
	if (vec == nullptr)
		botHasDestination = false;
	else {
		botHasDestination = true;
		botDestination = *vec;
	}
}

Action* Player::getAction(unsigned int idx) {
	if (idx > 63) throw std::invalid_argument(errorOutOfRange);

	return &actions[idx];
}

MenuButton* Player::getMenuButton(unsigned int idx) {
	if (idx > 31) throw std::invalid_argument(errorOutOfRange);

	return &menuButtons[idx];
}

std::string Human::__tostring() const {
	char buf[16];
	sprintf(buf, "Human(%i)", getIndex());
	return buf;
}

int Human::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::humans) / sizeof(*this);
}

sol::table Human::getDataTable() const {
	int index = getIndex();

	if (!humanDataTables[index]) {
		humanDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *humanDataTables[index];
}

void Human::remove() const {
	int index = getIndex();

	subhook::ScopedHookRemove remove(&Hooks::deleteHumanHook);
	Engine::deleteHuman(index);

	if (humanDataTables[index]) {
		delete humanDataTables[index];
		humanDataTables[index] = nullptr;
	}
}

Player* Human::getPlayer() const {
	return playerID == -1 ? nullptr : &Engine::players[playerID];
}

void Human::setPlayer(Player* player) {
	playerID = player == nullptr ? -1 : player->getIndex();
}

Account* Human::getAccount() {
	return accountID == -1 ? nullptr : &Engine::accounts[accountID];
}

void Human::setAccount(Account* account) {
	accountID = account != nullptr ? account->getIndex() : -1;
}

Vehicle* Human::getVehicle() const {
	return vehicleID == -1 ? nullptr : &Engine::vehicles[vehicleID];
}

void Human::setVehicle(Vehicle* vehicle) {
	vehicleID = vehicle == nullptr ? -1 : vehicle->getIndex();
}

void Human::teleport(Vector* vec) {
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

		body = &Engine::bodies[bone->bodyID];
		body->pos.x += offX;
		body->pos.y += offY;
		body->pos.z += offZ;
	}
};

void Human::speak(const char* message, int distance) const {
	subhook::ScopedHookRemove remove(&Hooks::createEventMessageHook);
	Engine::createEventMessage(1, (char*)message, getIndex(), distance);
}

void Human::arm(int weapon, int magCount) const {
	Engine::scenarioArmHuman(getIndex(), weapon, magCount);
}

Bone* Human::getBone(unsigned int idx) {
	if (idx > 15) throw std::invalid_argument(errorOutOfRange);

	return &bones[idx];
}

RigidBody* Human::getRigidBody(unsigned int idx) const {
	if (idx > 15) throw std::invalid_argument(errorOutOfRange);

	return &Engine::bodies[bones[idx].bodyID];
}

InventorySlot* Human::getInventorySlot(unsigned int idx) {
	if (idx > 6) throw std::invalid_argument(errorOutOfRange);

	return &inventorySlots[idx];
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
	subhook::ScopedHookRemove remove(&Hooks::linkItemHook);
	return Engine::linkItem(childItem->getIndex(), -1, getIndex(), slot);
}

void Human::applyDamage(int bone, int damage) const {
	subhook::ScopedHookRemove remove(&Hooks::humanApplyDamageHook);
	Engine::humanApplyDamage(getIndex(), bone, 0, damage);
}

std::string ItemType::__tostring() const {
	char buf[16];
	sprintf(buf, "ItemType(%i)", getIndex());
	return buf;
}

int ItemType::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::itemTypes) / sizeof(*this);
}

bool ItemType::getCanMountTo(ItemType* parent) const {
	if (parent == nullptr) {
		throw std::invalid_argument("Cannot compare to nil parent");
	}

	return canMountTo[parent->getIndex()];
}

void ItemType::setCanMountTo(ItemType* parent, bool b) {
	if (parent == nullptr) {
		throw std::invalid_argument("Cannot compare to nil parent");
	}

	canMountTo[parent->getIndex()] = b;
}

std::string Item::__tostring() const {
	char buf[16];
	sprintf(buf, "Item(%i)", getIndex());
	return buf;
}

int Item::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::items) / sizeof(*this);
}

sol::table Item::getDataTable() const {
	int index = getIndex();

	if (!itemDataTables[index]) {
		itemDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *itemDataTables[index];
}

ItemType* Item::getType() { return &Engine::itemTypes[type]; }

void Item::setType(ItemType* itemType) {
	if (itemType == nullptr) {
		throw std::invalid_argument("Cannot set an item's type to nil");
	}

	type = itemType->getIndex();
}

void Item::remove() const {
	int index = getIndex();

	subhook::ScopedHookRemove remove(&Hooks::deleteItemHook);
	Engine::deleteItem(index);

	if (itemDataTables[index]) {
		delete itemDataTables[index];
		itemDataTables[index] = nullptr;
	}
}

Player* Item::getGrenadePrimer() const {
	return grenadePrimerID == -1 ? nullptr : &Engine::players[grenadePrimerID];
}

void Item::setGrenadePrimer(Player* player) {
	grenadePrimerID = player != nullptr ? player->getIndex() : -1;
}

Human* Item::getParentHuman() const {
	return parentHumanID == -1 ? nullptr : &Engine::humans[parentHumanID];
}

void Item::setParentHuman(Human* human) {
	parentHumanID = human == nullptr ? -1 : human->getIndex();
}

Item* Item::getParentItem() const {
	return parentItemID == -1 ? nullptr : &Engine::items[parentItemID];
}

void Item::setParentItem(Item* item) {
	parentItemID = item == nullptr ? -1 : item->getIndex();
}

RigidBody* Item::getRigidBody() const { return &Engine::bodies[bodyID]; }

Item* Item::getChildItem(unsigned int idx) const {
	if (idx >= numChildItems) throw std::invalid_argument(errorOutOfRange);

	return &Engine::items[childItemIDs[idx]];
}

Item* Item::getConnectedPhone() const {
	return connectedPhoneID == -1 ? nullptr : &Engine::items[connectedPhoneID];
}

void Item::setConnectedPhone(Item* item) {
	connectedPhoneID = item == nullptr ? -1 : item->getIndex();
}

Vehicle* Item::getVehicle() const {
	return vehicleID == -1 ? nullptr : &Engine::vehicles[vehicleID];
}

void Item::setVehicle(Vehicle* vehicle) {
	vehicleID = vehicle == nullptr ? -1 : vehicle->getIndex();
}

bool Item::mountItem(Item* childItem, unsigned int slot) const {
	subhook::ScopedHookRemove remove(&Hooks::linkItemHook);
	return Engine::linkItem(getIndex(), childItem->getIndex(), -1, slot);
}

bool Item::unmount() const {
	subhook::ScopedHookRemove remove(&Hooks::linkItemHook);
	return Engine::linkItem(getIndex(), -1, -1, 0);
}

Event* Item::update() const {
	subhook::ScopedHookRemove remove(&Hooks::createEventUpdateItemInfoHook);
	Engine::createEventUpdateItemInfo(getIndex());
	return &Engine::events[*Engine::numEvents - 1];
}

void Item::speak(const char* message, int distance) const {
	subhook::ScopedHookRemove remove(&Hooks::createEventMessageHook);
	Engine::createEventMessage(2, (char*)message, getIndex(), distance);
}

void Item::explode() const {
	subhook::ScopedHookRemove remove(&Hooks::grenadeExplosionHook);
	Engine::grenadeExplosion(getIndex());
}

void Item::setMemo(const char* memo) const {
	Engine::itemSetMemo(getIndex(), memo);
}

void Item::computerTransmitLine(unsigned int line) const {
	Engine::itemComputerTransmitLine(getIndex(), line);
}

void Item::computerIncrementLine() const {
	Engine::itemComputerIncrementLine(getIndex());
}

void Item::computerSetLine(unsigned int line, const char* newLine) {
	if (line >= 32) throw std::invalid_argument(errorOutOfRange);
	std::strncpy(computerLines[line], newLine, 63);
}

void Item::computerSetLineColors(unsigned int line, std::string_view colors) {
	if (line >= 32) throw std::invalid_argument(errorOutOfRange);
	std::memcpy(computerLineColors[line], colors.data(),
	            std::min(std::size_t(63), colors.size()));
}

void Item::computerSetColor(unsigned int line, unsigned int column,
                            unsigned char color) {
	if (line >= 32 || column >= 64) throw std::invalid_argument(errorOutOfRange);
	computerLineColors[line][column] = color;
}

void Item::cashAddBill(int position, int value) const {
	if (value >= 8) throw std::invalid_argument(errorOutOfRange);
	Engine::itemCashAddBill(getIndex(), position, value);
}

void Item::cashRemoveBill(int position) const {
	Engine::itemCashRemoveBill(getIndex(), position);
}

int Item::cashGetBillValue() const {
	return Engine::itemCashGetBillValue(getIndex());
}

std::string VehicleType::__tostring() const {
	char buf[16];
	sprintf(buf, "VehicleType(%i)", getIndex());
	return buf;
}

int VehicleType::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::vehicleTypes) / sizeof(*this);
}

std::string Vehicle::__tostring() const {
	char buf[16];
	sprintf(buf, "Vehicle(%i)", getIndex());
	return buf;
}

int Vehicle::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::vehicles) / sizeof(*this);
}

VehicleType* Vehicle::getType() { return &Engine::vehicleTypes[type]; }

void Vehicle::setType(VehicleType* vehicleType) {
	if (vehicleType == nullptr) {
		throw std::invalid_argument("Cannot set a vehicle's type to nil");
	}

	type = vehicleType->getIndex();
}

sol::table Vehicle::getDataTable() const {
	int index = getIndex();

	if (!vehicleDataTables[index]) {
		vehicleDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *vehicleDataTables[index];
}

Event* Vehicle::updateType() const {
	Engine::createEventCreateVehicle(getIndex());
	return &Engine::events[*Engine::numEvents - 1];
}

Event* Vehicle::updateDestruction(int updateType, int partID, Vector* pos,
                                  Vector* normal) const {
	subhook::ScopedHookRemove remove(&Hooks::createEventUpdateVehicleHook);
	Engine::createEventUpdateVehicle(getIndex(), updateType, partID, pos, normal);
	return &Engine::events[*Engine::numEvents - 1];
}

void Vehicle::remove() const {
	int index = getIndex();

	subhook::ScopedHookRemove remove(&Hooks::deleteVehicleHook);
	Engine::deleteVehicle(index);

	if (vehicleDataTables[index]) {
		delete vehicleDataTables[index];
		vehicleDataTables[index] = nullptr;
	}
}

Player* Vehicle::getLastDriver() const {
	return lastDriverPlayerID == -1 ? nullptr
	                                : &Engine::players[lastDriverPlayerID];
}

RigidBody* Vehicle::getRigidBody() const { return &Engine::bodies[bodyID]; }

TrafficCar* Vehicle::getTrafficCar() const {
	return trafficCarID == -1 ? nullptr : &Engine::trafficCars[trafficCarID];
};

void Vehicle::setTrafficCar(TrafficCar* trafficCar) {
	trafficCarID = trafficCar == nullptr ? -1 : trafficCar->getIndex();
};

bool Vehicle::getIsWindowBroken(unsigned int idx) const {
	if (idx >= 8) throw std::invalid_argument(errorOutOfRange);

	return windowStates[idx];
}

void Vehicle::setIsWindowBroken(unsigned int idx, bool b) {
	if (idx >= 8) throw std::invalid_argument(errorOutOfRange);

	windowStates[idx] = b;
}

Player* Bullet::getPlayer() const {
	return playerID == -1 ? nullptr : &Engine::players[playerID];
}

std::string RigidBody::__tostring() const {
	char buf[16];
	sprintf(buf, "RigidBody(%i)", getIndex());
	return buf;
}

int RigidBody::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::bodies) / sizeof(*this);
}

sol::table RigidBody::getDataTable() const {
	int index = getIndex();

	if (!bodyDataTables[index]) {
		bodyDataTables[index] = new sol::table(lua->lua_state(), sol::create);
	}

	return *bodyDataTables[index];
}

Bond* RigidBody::bondTo(RigidBody* other, Vector* thisLocalPos,
                        Vector* otherLocalPos) const {
	int id = Engine::createBondRigidBodyToRigidBody(getIndex(), other->getIndex(),
	                                                thisLocalPos, otherLocalPos);
	return id == -1 ? nullptr : &Engine::bonds[id];
}

Bond* RigidBody::bondRotTo(RigidBody* other) const {
	int id =
	    Engine::createBondRigidBodyRotRigidBody(getIndex(), other->getIndex());
	return id == -1 ? nullptr : &Engine::bonds[id];
}

Bond* RigidBody::bondToLevel(Vector* localPos, Vector* globalPos) const {
	int id = Engine::createBondRigidBodyToLevel(getIndex(), localPos, globalPos);
	return id == -1 ? nullptr : &Engine::bonds[id];
}

void RigidBody::collideLevel(Vector* localPos, Vector* normal, float a, float b,
                             float c, float d) const {
	Engine::addCollisionRigidBodyOnLevel(getIndex(), localPos, normal, a, b, c,
	                                     d);
}

Item* InventorySlot::getPrimaryItem() const {
	if (count < 1) {
		return nullptr;
	}

	return primaryItemID == -1 ? nullptr : &Engine::items[primaryItemID];
};

Item* InventorySlot::getSecondaryItem() const {
	if (count < 2) {
		return nullptr;
	}

	return secondaryItemID == -1 ? nullptr : &Engine::items[secondaryItemID];
};

std::string Bond::__tostring() const {
	char buf[16];
	sprintf(buf, "Bond(%i)", getIndex());
	return buf;
}

int Bond::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::bonds) / sizeof(*this);
}

RigidBody* Bond::getBody() const { return &Engine::bodies[bodyID]; }

RigidBody* Bond::getOtherBody() const { return &Engine::bodies[otherBodyID]; }

void Bond::remove() const {
	int index = getIndex();
	Engine::deleteBond(index);
}

std::string Street::__tostring() const {
	char buf[16];
	sprintf(buf, "Street(%i)", getIndex());
	return buf;
}

int Street::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::streets) / sizeof(*this);
}

StreetIntersection* Street::getIntersectionA() const {
	return &Engine::streetIntersections[intersectionA];
}

StreetIntersection* Street::getIntersectionB() const {
	return &Engine::streetIntersections[intersectionB];
}

StreetLane* Street::getLane(unsigned int idx) {
	if (idx >= numLanes) throw std::invalid_argument(errorOutOfRange);

	return &lanes[idx];
}

std::string StreetIntersection::__tostring() const {
	char buf[32];
	sprintf(buf, "StreetIntersection(%i)", getIndex());
	return buf;
}

int StreetIntersection::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::streetIntersections) /
	       sizeof(*this);
}

Street* StreetIntersection::getStreetEast() const {
	return streetEast == -1 ? nullptr : &Engine::streets[streetEast];
}

Street* StreetIntersection::getStreetSouth() const {
	return streetSouth == -1 ? nullptr : &Engine::streets[streetSouth];
}

Street* StreetIntersection::getStreetWest() const {
	return streetWest == -1 ? nullptr : &Engine::streets[streetWest];
}

Street* StreetIntersection::getStreetNorth() const {
	return streetNorth == -1 ? nullptr : &Engine::streets[streetNorth];
}

std::string TrafficCar::__tostring() const {
	char buf[24];
	sprintf(buf, "TrafficCar(%i)", getIndex());
	return buf;
}

int TrafficCar::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::trafficCars) / sizeof(*this);
}

VehicleType* TrafficCar::getType() { return &Engine::vehicleTypes[type]; }

void TrafficCar::setType(VehicleType* vehicleType) {
	if (vehicleType == nullptr) {
		throw std::invalid_argument("Cannot set a traffic car's type to nil");
	}

	type = vehicleType->getIndex();
}

Human* TrafficCar::getHuman() const {
	return humanID == -1 ? nullptr : &Engine::humans[humanID];
}

void TrafficCar::setHuman(Human* human) {
	humanID = human == nullptr ? -1 : human->getIndex();
}

Vehicle* TrafficCar::getVehicle() const {
	return vehicleID == -1 ? nullptr : &Engine::vehicles[vehicleID];
}

void TrafficCar::setVehicle(Vehicle* vehicle) {
	vehicleID = vehicle == nullptr ? -1 : vehicle->getIndex();
}

VehicleType* ShopCar::getType() { return &Engine::vehicleTypes[type]; }

void ShopCar::setType(VehicleType* vehicleType) {
	if (vehicleType == nullptr) {
		throw std::invalid_argument("Cannot set a shop car's type to nil");
	}

	type = vehicleType->getIndex();
}

std::string Building::__tostring() const {
	char buf[24];
	sprintf(buf, "Building(%i)", getIndex());
	return buf;
}

int Building::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::buildings) / sizeof(*this);
}

ShopCar* Building::getShopCar(unsigned int idx) {
	if (idx > 15) throw std::invalid_argument(errorOutOfRange);

	return &shopCars[idx];
}

std::string Event::__tostring() const {
	char buf[16];
	sprintf(buf, "Event(%i)", getIndex());
	return buf;
}

int Event::getIndex() const {
	return ((uintptr_t)this - (uintptr_t)Engine::events) / sizeof(*this);
}
