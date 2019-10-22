#pragma once
#include "engine.h"

#define RESET_REASON_BOOT 0
#define RESET_REASON_ENGINECALL 1
#define RESET_REASON_LUARESET 2
#define RESET_REASON_LUACALL 3

extern HMODULE DllHandle;

extern bool initialized;
extern bool shouldReset;

extern sol::state* lua;
extern std::string hookMode;

extern std::queue<std::string> consoleQueue;

enum LuaRequestType {
	post
};

struct LuaHTTPRequest {
	LuaRequestType type;
	std::string host;
	unsigned short port;
	std::string path;
	std::string contentType;
	std::string body;
	httplib::Headers headers;
};

extern std::queue<LuaHTTPRequest> requestQueue;

DWORD WINAPI HTTPThread(HMODULE hModule);

void printLuaError(sol::error* err);
bool noLuaCallError(sol::protected_function_result* res);
bool noLuaCallError(sol::load_result* res);
void hookAndReset(int reason);

DWORD WINAPI ConsoleThread(HMODULE hModule);
void luaInit(bool redo = false);

void l_printAppend(const char* str);
void l_flagStateForReset(const char* mode);

Vector l_Vector();
Vector l_Vector_3f(float x, float y, float z);
RotMatrix l_RotMatrix(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

void l_http_post(const char* host, int port, const char* path, sol::table headers, const char* body, const char* contentType);

void l_event_sound(int soundType, Vector* pos, float volume, float pitch);
void l_event_soundSimple(int soundType, Vector* pos);
void l_event_explosion(Vector* pos);
void l_event_bulletHit(int hitType, Vector* pos, Vector* normal);

sol::table l_physics_lineIntersectLevel(Vector* posA, Vector* posB);
sol::table l_physics_lineIntersectHuman(Human* man, Vector* posA, Vector* posB);
sol::table l_physics_lineIntersectVehicle(Vehicle* vcl, Vector* posA, Vector* posB);
void l_physics_garbageCollectBullets();

sol::table l_itemTypes_getAll();
ItemType* l_itemTypes_getByIndex(unsigned int idx);

int l_items_getCount();
sol::table l_items_getAll();
Item* l_items_getByIndex(unsigned int idx);
Item* l_items_create(int itemType, Vector* pos, RotMatrix* rot);
Item* l_items_createVel(int itemType, Vector* pos, Vector* vel, RotMatrix* rot);
void lua_items_createRope(Vector* pos, RotMatrix* rot);

sol::table l_vehicles_getAll();
Vehicle* l_vehicles_getByIndex(unsigned int idx);
Vehicle* l_vehicles_create(int type, Vector* pos, RotMatrix* rot, int color);
Vehicle* l_vehicles_createVel(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
void l_vehicles_createTraffic(int density);

void l_chat_announce(const char* message);
void l_chat_tellAdmins(const char* message);
void l_chat_addRaw(int type, const char* message, int speakerID, int distance);

sol::table l_accounts_getAll();
Account* l_accounts_getByPhone(int phone);

sol::table l_players_getAll();
Player* l_players_getByPhone(int phone);
sol::table l_players_getNonBots();
Player* l_players_getByIndex(unsigned int idx);
Player* l_players_createBot();

sol::table l_humans_getAll();
Human* l_humans_getByIndex(unsigned int idx);
Human* l_humans_create(Vector* pos, RotMatrix* rot, Player* ply);

sol::table l_bullets_getAll();

int l_rigidBodies_getCount();
sol::table l_rigidBodies_getAll();
RigidBody* l_rigidBodies_getByIndex(unsigned int idx);

void l_os_setClipboard(std::string s);
sol::table l_os_listDirectory(const char* path);