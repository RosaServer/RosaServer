#pragma once
#include "structs.h"

struct Version
{
	unsigned int major;
	unsigned int build;
};

extern int* gameType;
extern char* mapName;
extern char* loadedMapName;
extern int* gameState;
extern int* gameTimer;
extern unsigned int* sunTime;

extern RayCastResult* lineIntersectResult;

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

typedef void (*void_func)();
typedef void (*void_index_func)(int id);

extern void_func resetgame;

extern void_func logicsimulation;
extern void_func logicsimulation_race;
extern void_func logicsimulation_round;
extern void_func logicsimulation_world;
extern void_func logicsimulation_terminator;
extern void_func logicsimulation_coop;
extern void_func logicsimulation_versus;

extern void_func physicssimulation;
typedef int (*recvpacket_func)();
extern recvpacket_func recvpacket;
extern void_func sendpacket;
extern void_func bulletsimulation;
extern void_func bullettimetolive;

typedef int (*createaccount_jointicket_func)(int identifier, unsigned int ticket);
extern createaccount_jointicket_func createaccount_jointicket;
typedef void (*server_sendconnectreponse_func)(unsigned int address, unsigned int port, const char* message);
extern server_sendconnectreponse_func server_sendconnectreponse;

typedef void (*scenario_armhuman_func)(int human, int weapon, int magCount);
extern scenario_armhuman_func scenario_armhuman;

typedef int (*linkitem_func)(int itemID, int childItemID, int parentHumanID, int slot);
extern linkitem_func linkitem;

typedef void (*human_applydamage_func)(int humanID, int bone, int unk, int damage);
extern human_applydamage_func human_applydamage;

typedef void (*human_collisionvehicle_func)(int humanID, int vehicleID);
extern human_collisionvehicle_func human_collisionvehicle;

extern void_index_func human_grabbing;

extern void_index_func grenadeexplosion;

typedef int (*chat_func)(int playerID, char* message);
extern chat_func chat;
extern void_index_func playerai;
extern void_index_func playerdeathtax;

/*
	Object Handling
*/

typedef int (*createplayer_func)();
extern createplayer_func createplayer;
extern void_index_func deleteplayer;

typedef int (*createhuman_func)(Vector* pos, RotMatrix* rot, int playerID);
extern createhuman_func createhuman;
extern void_index_func deletehuman;

typedef int (*createitem_func)(int type, Vector* pos, Vector* vel, RotMatrix* rot);
extern createitem_func createitem;
extern void_index_func deleteitem;

typedef int (*createrope_func)(Vector* pos, RotMatrix* rot);
extern createrope_func createrope;

typedef int (*createobject_func)(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
extern createobject_func createobject;
extern void_index_func deleteobject;

/*
	Events
*/

typedef void (*createevent_message_func)(int type, char* message, int speakerID, int distance);
extern createevent_message_func createevent_message;
// Sends team, active, isBot, humanID, skinColor, hair, gender, head, necklace, eyeColor, tieColor, suitColor, shirtColor, hairColor, name
extern void_index_func createevent_updateplayer;
// Sends money, stocks, phoneNumber
extern void_index_func createevent_updateplayer_finance;
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
typedef void (*createevent_updateobject_func)(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal);
extern createevent_updateobject_func createevent_updateobject;
typedef void (*createevent_sound_func)(int soundType, Vector* pos, float volume, float pitch);
extern createevent_sound_func createevent_sound;
typedef void (*createevent_explosion_func)(int type, Vector* pos);
extern createevent_explosion_func createevent_explosion;
/*
hitType:
0 = bullet hole
1 = hit body
2 = hit car
3 = blood drip
*/
typedef void (*createevent_bullethit_func)(int unk, int hitType, Vector* pos, Vector* normal);
extern createevent_bullethit_func createevent_bullethit;

/*
	Math
*/

typedef int (*lineintersectlevel_func)(Vector* posA, Vector* posB);
extern lineintersectlevel_func lineintersectlevel;

typedef int (*lineintersecthuman_func)(int humanID, Vector* posA, Vector* posB);
extern lineintersecthuman_func lineintersecthuman;

typedef int (*lineintersectobject_func)(int vehicleID, Vector* posA, Vector* posB);
extern lineintersectobject_func lineintersectobject;