#pragma once
#include "structs.h"
#include "subhook.h"

extern subhook::Hook resetgame_hook;
void h_resetgame();

extern subhook::Hook logicsimulation_hook;
void h_logicsimulation();
extern subhook::Hook logicsimulation_race_hook;
void h_logicsimulation_race();
extern subhook::Hook logicsimulation_round_hook;
void h_logicsimulation_round();
extern subhook::Hook logicsimulation_world_hook;
void h_logicsimulation_world();
extern subhook::Hook logicsimulation_terminator_hook;
void h_logicsimulation_terminator();
extern subhook::Hook logicsimulation_coop_hook;
void h_logicsimulation_coop();
extern subhook::Hook logicsimulation_versus_hook;
void h_logicsimulation_versus();
extern subhook::Hook logic_playeractions_hook;
void h_logic_playeractions(int playerID);

extern subhook::Hook physicssimulation_hook;
void h_physicssimulation();
extern subhook::Hook serverrecv_hook;
int h_serverrecv();
extern subhook::Hook serversend_hook;
void h_serversend();
extern subhook::Hook bulletsimulation_hook;
void h_bulletsimulation();

extern subhook::Hook saveaccountsserver_hook;
void h_saveaccountsserver();

extern subhook::Hook createaccount_jointicket_hook;
int h_createaccount_jointicket(int identifier, unsigned int ticket);
extern subhook::Hook server_sendconnectreponse_hook;
void h_server_sendconnectreponse(unsigned int address, unsigned int port, const char* message);

extern subhook::Hook createplayer_hook;
int h_createplayer();
extern subhook::Hook deleteplayer_hook;
void h_deleteplayer(int playerID);
extern subhook::Hook createhuman_hook;
int h_createhuman(Vector* pos, RotMatrix* rot, int playerID);
extern subhook::Hook deletehuman_hook;
void h_deletehuman(int humanID);
extern subhook::Hook createitem_hook;
int h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot);
extern subhook::Hook deleteitem_hook;
void h_deleteitem(int itemID);
extern subhook::Hook createobject_hook;
int h_createobject(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
extern subhook::Hook deleteobject_hook;
void h_deleteobject(int vehicleID);

extern subhook::Hook linkitem_hook;
int h_linkitem(int itemID, int childItemID, int parentHumanID, int slot);
extern subhook::Hook item_computerinput_hook;
void h_item_computerinput(int itemID, unsigned int character);
extern subhook::Hook human_applydamage_hook;
void h_human_applydamage(int humanID, int bone, int unk, int damage);
extern subhook::Hook human_collisionvehicle_hook;
void h_human_collisionvehicle(int humanID, int vehicleID);
extern subhook::Hook human_grabbing_hook;
void h_human_grabbing(int humanID);
extern subhook::Hook grenadeexplosion_hook;
void h_grenadeexplosion(int itemID);
extern subhook::Hook server_playermessage_hook;
int h_server_playermessage(int playerID, char* message);
extern subhook::Hook playerai_hook;
void h_playerai(int playerID);
extern subhook::Hook playerdeathtax_hook;
void h_playerdeathtax(int playerID);

extern subhook::Hook addcollision_rigidbody_rigidbody_hook;
void h_addcollision_rigidbody_rigidbody(int aBodyID, int bBodyID, Vector* aLocalPos, Vector* bLocalPos, Vector* normal, float, float, float, float);

extern subhook::Hook createevent_message_hook;
void h_createevent_message(int speakerType, char* message, int speakerID, int distance);
extern subhook::Hook createevent_updateplayer_hook;
void h_createevent_updateplayer(int id);
extern subhook::Hook createevent_updateplayer_finance_hook;
void h_createevent_updateplayer_finance(int id);
//extern subhook::Hook createevent_updateitem_hook;
//void h_createevent_updateitem(int id);
extern subhook::Hook createevent_updateobject_hook;
void h_createevent_updateobject(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal);
//extern subhook::Hook createevent_sound_hook;
//void h_createevent_sound(int soundType, Vector* pos, float volume, float pitch);
extern subhook::Hook createevent_bullethit_hook;
void h_createevent_bullethit(int unk, int hitType, Vector* pos, Vector* normal);

extern subhook::Hook lineintersecthuman_hook;
int h_lineintersecthuman(int humanID, Vector* posA, Vector* posB);