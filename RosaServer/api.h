#pragma once
#include "engine.h"
#include "hooks.h"
#include "sol/sol.hpp"

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../cpp-httplib/httplib.h"

#define LUA_ENTRY_FILE "main/init.lua"
#define LUA_PREFIX "\033[34;1;4m[RosaServer/Lua]\033[0m "
#define RS_PREFIX "\033[35;1;4m[RosaServer]\033[0m "
#define SUBROSA_PREFIX "\033[31;1;4m[Sub Rosa]\033[0m "

#define RESET_REASON_BOOT 0
#define RESET_REASON_ENGINECALL 1
#define RESET_REASON_LUARESET 2
#define RESET_REASON_LUACALL 3

extern bool initialized;
extern bool shouldReset;

extern sol::state* lua;
extern std::string hookMode;

extern sol::table* playerDataTables[maxNumberOfPlayers];
extern sol::table* humanDataTables[maxNumberOfHumans];
extern sol::table* itemDataTables[maxNumberOfItems];
extern sol::table* vehicleDataTables[maxNumberOfVehicles];
extern sol::table* bodyDataTables[maxNumberOfRigidBodies];

enum LuaRequestType { get, post };

struct LuaHTTPRequest {
	LuaRequestType type;
	std::string scheme;
	std::string path;
	std::shared_ptr<sol::protected_function> callback;
	std::string contentType;
	std::string body;
	httplib::Headers headers;
};

struct LuaHTTPResponse {
	std::shared_ptr<sol::protected_function> callback;
	bool responded;
	int status;
	std::string body;
	httplib::Headers headers;
};

extern std::mutex stateResetMutex;

void printLuaError(sol::error* err);
bool noLuaCallError(sol::protected_function_result* res);
bool noLuaCallError(sol::load_result* res);
void hookAndReset(int reason);

void defineThreadSafeAPIs(sol::state* state);
void luaInit(bool redo = false);

namespace Lua {
void print(sol::variadic_args va, sol::this_state s);
void flagStateForReset(const char* mode);

Vector Vector_();
Vector Vector_3f(float x, float y, float z);
RotMatrix RotMatrix_(float x1, float y1, float z1, float x2, float y2, float z2,
                     float x3, float y3, float z3);

namespace http {
void get(const char* scheme, const char* path, sol::table headers,
         sol::protected_function callback);
void post(const char* scheme, const char* path, sol::table headers,
          std::string body, const char* contentType,
          sol::protected_function callback);

sol::object getSync(const char* scheme, const char* path, sol::table headers,
                    sol::this_state s);
sol::object postSync(const char* scheme, const char* path, sol::table headers,
                     std::string body, const char* contentType,
                     sol::this_state s);
};  // namespace http

namespace event {
void sound(int soundType, Vector* pos, float volume, float pitch);
void soundSimple(int soundType, Vector* pos);
void explosion(Vector* pos);
void bulletHit(int hitType, Vector* pos, Vector* normal);
};  // namespace event

namespace physics {
sol::table lineIntersectLevel(Vector* posA, Vector* posB);
sol::table lineIntersectHuman(Human* man, Vector* posA, Vector* posB);
sol::table lineIntersectVehicle(Vehicle* vcl, Vector* posA, Vector* posB);
sol::object lineIntersectTriangle(Vector* outPos, Vector* normal, Vector* posA,
                                  Vector* posB, Vector* triA, Vector* triB,
                                  Vector* triC, sol::this_state s);
void garbageCollectBullets();
};  // namespace physics

namespace itemTypes {
int getCount();
sol::table getAll();
ItemType* getByIndex(sol::table self, unsigned int idx);
};  // namespace itemTypes

namespace items {
int getCount();
sol::table getAll();
Item* getByIndex(sol::table self, unsigned int idx);
Item* create(int itemType, Vector* pos, RotMatrix* rot);
Item* createVel(int itemType, Vector* pos, Vector* vel, RotMatrix* rot);
Item* createRope(Vector* pos, RotMatrix* rot);
};  // namespace items

namespace vehicles {
int getCount();
sol::table getAll();
Vehicle* getByIndex(sol::table self, unsigned int idx);
Vehicle* create(int type, Vector* pos, RotMatrix* rot, int color);
Vehicle* createVel(int type, Vector* pos, Vector* vel, RotMatrix* rot,
                   int color);
};  // namespace vehicles

namespace chat {
void announce(const char* message);
void tellAdmins(const char* message);
void addRaw(int type, const char* message, int speakerID, int distance);
};  // namespace chat

namespace accounts {
void save();
int getCount();
sol::table getAll();
Account* getByPhone(int phone);
Account* getByIndex(sol::table self, unsigned int idx);
};  // namespace accounts

namespace players {
int getCount();
sol::table getAll();
Player* getByPhone(int phone);
sol::table getNonBots();
Player* getByIndex(sol::table self, unsigned int idx);
Player* createBot();
};  // namespace players

namespace humans {
int getCount();
sol::table getAll();
Human* getByIndex(sol::table self, unsigned int idx);
Human* create(Vector* pos, RotMatrix* rot, Player* ply);
};  // namespace humans

namespace bullets {
unsigned int getCount();
sol::table getAll();
};  // namespace bullets

namespace rigidBodies {
int getCount();
sol::table getAll();
RigidBody* getByIndex(sol::table self, unsigned int idx);
};  // namespace rigidBodies

namespace bonds {
int getCount();
sol::table getAll();
Bond* getByIndex(sol::table self, unsigned int idx);
};  // namespace bonds

namespace streets {
int getCount();
sol::table getAll();
Street* getByIndex(sol::table self, unsigned int idx);
};  // namespace streets

namespace intersections {
int getCount();
sol::table getAll();
StreetIntersection* getByIndex(sol::table self, unsigned int idx);
};  // namespace intersections

namespace os {
sol::table listDirectory(const char* path, sol::this_state s);
bool createDirectory(const char* path);
double realClock();
void exit();
void exitCode(int code);
};  // namespace os
};  // namespace Lua

std::string addressFromInteger(unsigned int address);