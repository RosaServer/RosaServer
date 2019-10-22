#pragma once
#include "pch.h"
#include "structs.h"

struct Version {
	unsigned int major;
	unsigned int build;
};

extern int* gameType;
extern char* mapName;
extern int* gameState;
extern int* gameTimer;

extern RayCastResult* lineIntersectResult;

extern unsigned int* sunTime;

extern Connection* connections;
extern Account* accounts;
extern Player* players;
extern Human* humans;
extern Vehicle* vehicles;
extern ItemType* itemTypes;
extern Item* items;
extern Bullet* bullets;
extern RigidBody* bodies;

extern unsigned int* numConnections;
extern unsigned int* numBullets;

/*
	Misc
*/

typedef void(__cdecl* playerai_func)(int playerID);
extern playerai_func playerai;

typedef int(__cdecl* recvpacket_func)();
extern recvpacket_func recvpacket;

typedef void(__cdecl* void_func)();
typedef void(__cdecl* void_index_func)(int id);

extern void_func rigidbodysimulation;
extern void_func objectsimulation;
extern void_func itemsimulation;
extern void_func humansimulation;

extern void_func logicsimulation;
extern void_func logicsimulation_race;
extern void_func logicsimulation_round;
extern void_func logicsimulation_world;
extern void_func logicsimulation_terminator;
extern void_func logicsimulation_coop;
extern void_func logicsimulation_versus;

extern void_func sendpacket;
extern void_func bulletsimulation;
extern void_func bullettimetolive;
extern void_func resetgame;

extern void_index_func scenario_createtraffic3;

typedef void(__cdecl* armhuman_func)(int human, int weapon, int magCount);
extern armhuman_func scenario_armhuman;

typedef BOOL(__cdecl* grabitem_func)(int itemID, int childItemID, int parentHumanID, int slot);
extern grabitem_func linkitem;

typedef int(__cdecl* chat_func)(int playerID, char* message);
extern chat_func chat;

/*
	Object Handling
*/

extern void_func createlevel;

typedef int(__cdecl* createplayer_func)();
extern createplayer_func createplayer;
extern void_index_func deleteplayer;
extern void_index_func playerdeathtax;

typedef int(__cdecl* createhuman_func)(Vector* pos, RotMatrix* rot, int playerID);
extern createhuman_func createhuman;
extern void_index_func deletehuman;

typedef void(__cdecl* human_applydamage_func)(int humanID, int bone, int unk, int damage);
extern human_applydamage_func human_applydamage;

typedef int(__cdecl* createitem_func)(int type, Vector* pos, Vector* vel, RotMatrix* rot);
extern createitem_func createitem;
extern void_index_func deleteitem;

typedef int(__cdecl* createrope_func)(Vector* pos, RotMatrix* rot);
extern createrope_func createrope;

typedef int(__cdecl* createvehicle_func)(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
extern createvehicle_func createvehicle;
extern void_index_func deleteobject;

extern void_index_func grenadeexplosion;

/*
	Events
*/

typedef void(__cdecl* createevent_message_func)(int type, char* message, int speakerID, int distance);
extern createevent_message_func createevent_message;

// Sends team, active, isBot, humanID, skinColor, hair, gender, head, necklace, eyeColor, tieColor, suitColor, shirtColor, hairColor, name
extern void_index_func createevent_updateplayer;
// Sends money, stocks, phoneNumber
extern void_index_func createevent_updateplayer_finance;
// Sends active, playerID
extern void_index_func createevent_updatehuman;
// Sends active, type, parentHumanID, parentItemID, parentSlot
extern void_index_func createevent_updateitem;
// Sends type, color
extern void_index_func createevent_createobject;

/*
updateType:
0 = window
1 = tire
2 = body
*/
typedef void(__cdecl* createevent_updateobject_func)(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal);
extern createevent_updateobject_func createevent_updateobject;

typedef void(__cdecl* createevent_sound_func)(int soundType, Vector* pos, float volume, float pitch);
extern createevent_sound_func createevent_sound;

typedef void(__cdecl* createevent_explosion_func)(int type, Vector* pos);
extern createevent_explosion_func createevent_explosion;

typedef void(__cdecl* createevent_updatedoor_func)(int team, BOOL isOpen);
extern createevent_updatedoor_func createevent_updatedoor;

/*
hitType:
0 = bullet hole
1 = hit body
2 = hit car
3 = blood drip
*/
typedef void(__cdecl* createevent_bullethit_func)(int unk, int hitType, Vector* pos, Vector* normal);
extern createevent_bullethit_func createevent_bullethit;

/*
	Math
*/

typedef BOOL(__cdecl* lineintersectlevel_func)(Vector* posA, Vector* posB);
extern lineintersectlevel_func lineintersectlevel;

typedef BOOL(__cdecl* lineintersecthuman_func)(int humanID, Vector* posA, Vector* posB);
extern lineintersecthuman_func lineintersecthuman;

typedef BOOL(__cdecl* lineintersectobject_func)(int vehicleID, Vector* posA, Vector* posB);
extern lineintersectobject_func lineintersectobject;