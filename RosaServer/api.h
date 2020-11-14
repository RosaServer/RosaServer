#pragma once
#include "engine.h"
#include "hooks.h"
#include "sol/sol.hpp"

#include <queue>
#include <thread>
#include <mutex>
#include <memory>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../cpp-httplib/httplib.h"

#define RESET_REASON_BOOT 0
#define RESET_REASON_ENGINECALL 1
#define RESET_REASON_LUARESET 2
#define RESET_REASON_LUACALL 3

extern bool initialized;
extern bool shouldReset;

extern sol::state* lua;
extern std::string hookMode;

extern sol::table* playerDataTables[MAXNUMOFPLAYERS];
extern sol::table* humanDataTables[MAXNUMOFHUMANS];
extern sol::table* itemDataTables[MAXNUMOFITEMS];
extern sol::table* vehicleDataTables[MAXNUMOFVEHICLES];
extern sol::table* bodyDataTables[MAXNUMOFRIGIDBODIES];

enum LuaRequestType
{
	get,
	post
};

struct LuaHTTPRequest
{
	LuaRequestType type;
	std::string scheme;
	std::string path;
	std::shared_ptr<sol::protected_function> callback;
	std::string contentType;
	std::string body;
	httplib::Headers headers;
};

struct LuaHTTPResponse
{
	std::shared_ptr<sol::protected_function> callback;
	bool responded;
	int status;
	std::string body;
	httplib::Headers headers;
};

extern std::mutex stateResetMutex;
extern std::queue<LuaHTTPRequest> requestQueue;
extern std::mutex requestQueueMutex;
extern std::queue<LuaHTTPResponse> responseQueue;
extern std::mutex responseQueueMutex;

void HTTPThread();

void printLuaError(sol::error* err);
bool noLuaCallError(sol::protected_function_result* res);
bool noLuaCallError(sol::load_result* res);
void hookAndReset(int reason);

// void consoleThread();
void luaInit(bool redo = false);

void l_print(sol::variadic_args va);
void l_printAppend(const char* str);
void l_flagStateForReset(const char* mode);

Vector l_Vector();
Vector l_Vector_3f(float x, float y, float z);
RotMatrix l_RotMatrix(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

void l_http_get(const char* scheme, const char* path, sol::table headers, sol::protected_function callback);
void l_http_post(const char* scheme, const char* path, sol::table headers, const char* body, const char* contentType, sol::protected_function callback);

void l_event_sound(int soundType, Vector* pos, float volume, float pitch);
void l_event_soundSimple(int soundType, Vector* pos);
void l_event_explosion(Vector* pos);
void l_event_bulletHit(int hitType, Vector* pos, Vector* normal);

sol::table l_physics_lineIntersectLevel(Vector* posA, Vector* posB);
sol::table l_physics_lineIntersectHuman(Human* man, Vector* posA, Vector* posB);
sol::table l_physics_lineIntersectVehicle(Vehicle* vcl, Vector* posA, Vector* posB);
sol::object l_physics_lineIntersectTriangle(Vector* outPos, Vector* normal, Vector* posA, Vector* posB, Vector* triA, Vector* triB, Vector* triC, sol::this_state s);
void l_physics_garbageCollectBullets();

int l_itemTypes_getCount();
sol::table l_itemTypes_getAll();
ItemType* l_itemTypes_getByIndex(sol::table self, unsigned int idx);

int l_items_getCount();
sol::table l_items_getAll();
Item* l_items_getByIndex(sol::table self, unsigned int idx);
Item* l_items_create(int itemType, Vector* pos, RotMatrix* rot);
Item* l_items_createVel(int itemType, Vector* pos, Vector* vel, RotMatrix* rot);
Item* l_items_createRope(Vector* pos, RotMatrix* rot);

int l_vehicles_getCount();
sol::table l_vehicles_getAll();
Vehicle* l_vehicles_getByIndex(sol::table self, unsigned int idx);
Vehicle* l_vehicles_create(int type, Vector* pos, RotMatrix* rot, int color);
Vehicle* l_vehicles_createVel(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
//void l_vehicles_createTraffic(int density);

void l_chat_announce(const char* message);
void l_chat_tellAdmins(const char* message);
void l_chat_addRaw(int type, const char* message, int speakerID, int distance);

void l_accounts_save();
int l_accounts_getCount();
sol::table l_accounts_getAll();
Account* l_accounts_getByPhone(int phone);
Account* l_accounts_getByIndex(sol::table self, unsigned int idx);

int l_players_getCount();
sol::table l_players_getAll();
Player* l_players_getByPhone(int phone);
sol::table l_players_getNonBots();
Player* l_players_getByIndex(sol::table self, unsigned int idx);
Player* l_players_createBot();

int l_humans_getCount();
sol::table l_humans_getAll();
Human* l_humans_getByIndex(sol::table self, unsigned int idx);
Human* l_humans_create(Vector* pos, RotMatrix* rot, Player* ply);

unsigned int l_bullets_getCount();
sol::table l_bullets_getAll();

int l_rigidBodies_getCount();
sol::table l_rigidBodies_getAll();
RigidBody* l_rigidBodies_getByIndex(sol::table self, unsigned int idx);

int l_bonds_getCount();
sol::table l_bonds_getAll();
Bond* l_bonds_getByIndex(sol::table self, unsigned int idx);

sol::table l_os_listDirectory(const char* path);
bool l_os_createDirectory(const char* path);
double l_os_clock();

std::string addressFromInteger(unsigned int address);