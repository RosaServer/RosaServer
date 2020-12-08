#pragma once
#include "structs.h"
#include "subhook.h"

extern subhook::Hook subRosaPutsHook;
int h_subrosa_puts(const char* str);
extern subhook::Hook subRosa__printf_chkHook;
int h_subrosa___printf_chk(int flag, const char* format, ...);

extern subhook::Hook resetGameHook;
void h_resetgame();

extern subhook::Hook logicSimulationHook;
void h_logicsimulation();
extern subhook::Hook logicSimulationRaceHook;
void h_logicsimulation_race();
extern subhook::Hook logicSimulationRoundHook;
void h_logicsimulation_round();
extern subhook::Hook logicSimulationWorldHook;
void h_logicsimulation_world();
extern subhook::Hook logicSimulationTerminatorHook;
void h_logicsimulation_terminator();
extern subhook::Hook logicSimulationCoopHook;
void h_logicsimulation_coop();
extern subhook::Hook logicSimulationVersusHook;
void h_logicsimulation_versus();
extern subhook::Hook logicPlayerActionsHook;
void h_logic_playeractions(int playerID);

extern subhook::Hook physicsSimulationHook;
void h_physicssimulation();
extern subhook::Hook serverReceiveHook;
int h_serverrecv();
extern subhook::Hook serverSendHook;
void h_serversend();
extern subhook::Hook bulletSimulationHook;
void h_bulletsimulation();

extern subhook::Hook saveAccountsServerHook;
void h_saveaccountsserver();

extern subhook::Hook createAccountByJoinTicketHook;
int h_createaccount_jointicket(int identifier, unsigned int ticket);
extern subhook::Hook serverSendConnectResponseHook;
void h_server_sendconnectreponse(unsigned int address, unsigned int port,
                                 const char* message);

extern subhook::Hook createPlayerHook;
int h_createplayer();
extern subhook::Hook deletePlayerHook;
void h_deleteplayer(int playerID);
extern subhook::Hook createHumanHook;
int h_createhuman(Vector* pos, RotMatrix* rot, int playerID);
extern subhook::Hook deleteHumanHook;
void h_deletehuman(int humanID);
extern subhook::Hook createItemHook;
int h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot);
extern subhook::Hook deleteItemHook;
void h_deleteitem(int itemID);
extern subhook::Hook createVehicleHook;
int h_createobject(int type, Vector* pos, Vector* vel, RotMatrix* rot,
                   int color);
extern subhook::Hook deleteVehicleHook;
void h_deleteobject(int vehicleID);
extern subhook::Hook createRigidBodyHook;
int h_createrigidbody(int type, Vector* pos, RotMatrix* rot, Vector* vel,
                      Vector* scale, float mass);

extern subhook::Hook linkItemHook;
int h_linkitem(int itemID, int childItemID, int parentHumanID, int slot);
extern subhook::Hook itemComputerInputHook;
void h_item_computerinput(int itemID, unsigned int character);
extern subhook::Hook humanApplyDamageHook;
void h_human_applydamage(int humanID, int bone, int unk, int damage);
extern subhook::Hook humanCollisionVehicleHook;
void h_human_collisionvehicle(int humanID, int vehicleID);
extern subhook::Hook humanGrabbingHook;
void h_human_grabbing(int humanID);
extern subhook::Hook grenadeExplosionHook;
void h_grenadeexplosion(int itemID);
extern subhook::Hook serverPlayerMessageHook;
int h_server_playermessage(int playerID, char* message);
extern subhook::Hook playerAIHook;
void h_playerai(int playerID);
extern subhook::Hook playerDeathTaxHook;
void h_playerdeathtax(int playerID);

extern subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
void h_addcollision_rigidbody_rigidbody(int aBodyID, int bBodyID,
                                        Vector* aLocalPos, Vector* bLocalPos,
                                        Vector* normal, float, float, float,
                                        float);

extern subhook::Hook createEventMessageHook;
void h_createevent_message(int speakerType, char* message, int speakerID,
                           int distance);
extern subhook::Hook createEventUpdatePlayerHook;
void h_createevent_updateplayer(int id);
extern subhook::Hook createEventUpdatePlayerFinanceHook;
void h_createevent_updateplayer_finance(int id);
extern subhook::Hook createEventUpdateVehicleHook;
void h_createevent_updateobject(int vehicleID, int updateType, int partID,
                                Vector* pos, Vector* normal);
// extern subhook::Hook createevent_sound_hook;
// void h_createevent_sound(int soundType, Vector* pos, float volume, float
// pitch);
extern subhook::Hook createEventBulletHitHook;
void h_createevent_bullethit(int unk, int hitType, Vector* pos, Vector* normal);

extern subhook::Hook lineIntersectHumanHook;
int h_lineintersecthuman(int humanID, Vector* posA, Vector* posB);