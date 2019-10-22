#pragma once
#include "pch.h"
#include "structs.h"

void __cdecl h_createlevel();
int __cdecl h_createplayer();
void __cdecl h_deleteplayer(int playerID);
int __cdecl h_createhuman(Vector* pos, RotMatrix* rot, int playerID);
void __cdecl h_deletehuman(int humanID);
BOOL __cdecl h_linkitem(int itemID, int childItemID, int parentHumanID, int slot);
int __cdecl h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot);
int __cdecl h_createvehicle(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
void __cdecl h_grenadeexplosion(int itemID);
void __cdecl h_playerdeathtax(int playerID);
void __cdecl h_human_applydamage(int humanID, int bone, int unk, int damage);
int __cdecl h_chat(int playerID, char* message);
void __cdecl h_createevent_message(int type, char* message, int speakerID, int distance);
void __cdecl h_createevent_updateitem(int id);
void __cdecl h_createevent_updateplayer(int id);
void __cdecl h_createevent_updateplayer_finance(int id);
void __cdecl h_createevent_updateobject(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal);
void __cdecl h_createevent_sound(int soundType, Vector* pos, float volume, float pitch);
void __cdecl h_createevent_updatedoor(int team, BOOL isOpen);
void __cdecl h_createevent_bullethit(int unk, int hitType, Vector* pos, Vector* normal);
void __cdecl h_playerai(int playerID);
void __cdecl h_rigidbodysimulation();
void __cdecl h_objectsimulation();
void __cdecl h_itemsimulation();
void __cdecl h_humansimulation();
void __cdecl h_logicsimulation();
void __cdecl h_logicsimulation_race();
void __cdecl h_logicsimulation_round();
void __cdecl h_logicsimulation_world();
void __cdecl h_logicsimulation_terminator();
void __cdecl h_logicsimulation_coop();
void __cdecl h_logicsimulation_versus();
int __cdecl h_recvpacket();
void __cdecl h_sendpacket();
void __cdecl h_bulletsimulation();
void __cdecl h_resetgame();
void __cdecl h_scenario_createtraffic3(int density);