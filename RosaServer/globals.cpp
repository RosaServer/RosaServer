#include "api.h"
#include "engine.h"
#include "hooks.h"

bool initialized = false;
bool shouldReset = false;

sol::state* lua;
std::string hookMode;

sol::table* playerDataTables[MAXNUMOFPLAYERS];
sol::table* humanDataTables[MAXNUMOFHUMANS];
sol::table* itemDataTables[MAXNUMOFITEMS];
sol::table* vehicleDataTables[MAXNUMOFVEHICLES];
sol::table* bodyDataTables[MAXNUMOFRIGIDBODIES];

std::mutex stateResetMutex;
std::queue<LuaHTTPRequest> requestQueue;
std::mutex requestQueueMutex;
std::queue<LuaHTTPResponse> responseQueue;
std::mutex responseQueueMutex;

int* gameType;
char* mapName;
char* loadedMapName;
int* gameState;
int* gameTimer;
unsigned int* sunTime;

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
Street* streets;
StreetIntersection* streetIntersections;

unsigned int* numConnections;
unsigned int* numBullets;
unsigned int* numStreets;
unsigned int* numStreetIntersections;

subhook::Hook subrosa_puts_hook;
subrosa_puts_func subrosa_puts;
subhook::Hook subrosa___printf_chk_hook;
subrosa___printf_chk_func subrosa___printf_chk;

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
void_index_func item_computerincrementline;
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
subhook::Hook addcollision_rigidbody_rigidbody_hook;
addcollision_rigidbody_rigidbody_func addcollision_rigidbody_rigidbody;
addcollision_rigidbody_level_func addcollision_rigidbody_level;

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
subhook::Hook createrigidbody_hook;
createrigidbody_func createrigidbody;

subhook::Hook createevent_message_hook;
createevent_message_func createevent_message;
subhook::Hook createevent_updateplayer_hook;
void_index_func createevent_updateplayer;
subhook::Hook createevent_updateplayer_finance_hook;
void_index_func createevent_updateplayer_finance;
void_index_func createevent_createobject;
subhook::Hook createevent_updateobject_hook;
createevent_updateobject_func createevent_updateobject;
// subhook::Hook createevent_sound_hook;
createevent_sound_func createevent_sound;
createevent_explosion_func createevent_explosion;
subhook::Hook createevent_bullethit_hook;
createevent_bullethit_func createevent_bullethit;

lineintersectlevel_func lineintersectlevel;
subhook::Hook lineintersecthuman_hook;
lineintersecthuman_func lineintersecthuman;
lineintersectobject_func lineintersectobject;
lineintersecttriangle_func lineintersecttriangle;