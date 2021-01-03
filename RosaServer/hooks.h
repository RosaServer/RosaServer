#pragma once
#include "structs.h"
#include "subhook.h"

#include <set>
#include <unordered_map>

namespace Hooks {
enum EnableKeys {
	ResetGame,
	Logic,
	LogicRace,
	LogicRound,
	LogicWorld,
	LogicTerminator,
	LogicCoop,
	LogicVersus,
	PlayerActions,
	Physics,
	InPacket,
	SendPacket,
	PhysicsBullets,
	AccountsSave,
	AccountTicketBegin,
	SendConnectResponse,
	ItemLink,
	ItemComputerInput,
	HumanDamage,
	HumanCollisionVehicle,
	HumanGrabbing,
	GrenadeExplode,
	PlayerChat,
	PlayerAI,
	PlayerDeathTax,
	CollideBodies,
	BulletCreate,
	PlayerCreate,
	PlayerDelete,
	HumanCreate,
	HumanDelete,
	ItemCreate,
	ItemDelete,
	VehicleCreate,
	VehicleDelete,
	EventMessage,
	EventUpdatePlayer,
	EventUpdateVehicle,
	EventBullet,
	EventBulletHit,
	LineIntersectHuman,
	SIZE
};

extern const std::unordered_map<std::string, EnableKeys> enableNames;
extern bool enabledKeys[EnableKeys::SIZE];

extern subhook::Hook subRosaPutsHook;
int subRosaPuts(const char* str);
extern subhook::Hook subRosa__printf_chkHook;
int subRosa__printf_chk(int flag, const char* format, ...);

extern subhook::Hook resetGameHook;
void resetGame();

extern subhook::Hook logicSimulationHook;
void logicSimulation();
extern subhook::Hook logicSimulationRaceHook;
void logicSimulationRace();
extern subhook::Hook logicSimulationRoundHook;
void logicSimulationRound();
extern subhook::Hook logicSimulationWorldHook;
void logicSimulationWorld();
extern subhook::Hook logicSimulationTerminatorHook;
void logicSimulationTerminator();
extern subhook::Hook logicSimulationCoopHook;
void logicSimulationCoop();
extern subhook::Hook logicSimulationVersusHook;
void logicSimulationVersus();
extern subhook::Hook logicPlayerActionsHook;
void logicPlayerActions(int playerID);

extern subhook::Hook physicsSimulationHook;
void physicsSimulation();
extern subhook::Hook serverReceiveHook;
int serverReceive();
extern subhook::Hook serverSendHook;
void serverSend();
extern subhook::Hook bulletSimulationHook;
void bulletSimulation();

extern subhook::Hook saveAccountsServerHook;
void saveAccountsServer();

extern subhook::Hook createAccountByJoinTicketHook;
int createAccountByJoinTicket(int identifier, unsigned int ticket);
extern subhook::Hook serverSendConnectResponseHook;
void serverSendConnectResponse(unsigned int address, unsigned int port,
                               const char* message);

extern subhook::Hook createBulletHook;
int createBullet(int type, Vector* pos, Vector* vel, int playerID);
extern subhook::Hook createPlayerHook;
int createPlayer();
extern subhook::Hook deletePlayerHook;
void deletePlayer(int playerID);
extern subhook::Hook createHumanHook;
int createHuman(Vector* pos, RotMatrix* rot, int playerID);
extern subhook::Hook deleteHumanHook;
void deleteHuman(int humanID);
extern subhook::Hook createItemHook;
int createItem(int type, Vector* pos, Vector* vel, RotMatrix* rot);
extern subhook::Hook deleteItemHook;
void deleteItem(int itemID);
extern subhook::Hook createVehicleHook;
int createVehicle(int type, Vector* pos, Vector* vel, RotMatrix* rot,
                  int color);
extern subhook::Hook deleteVehicleHook;
void deleteVehicle(int vehicleID);
extern subhook::Hook createRigidBodyHook;
int createRigidBody(int type, Vector* pos, RotMatrix* rot, Vector* vel,
                    Vector* scale, float mass);

extern subhook::Hook linkItemHook;
int linkItem(int itemID, int childItemID, int parentHumanID, int slot);
extern subhook::Hook itemComputerInputHook;
void itemComputerInput(int itemID, unsigned int character);
extern subhook::Hook humanApplyDamageHook;
void humanApplyDamage(int humanID, int bone, int unk, int damage);
extern subhook::Hook humanCollisionVehicleHook;
void humanCollisionVehicle(int humanID, int vehicleID);
extern subhook::Hook humanGrabbingHook;
void humanGrabbing(int humanID);
extern subhook::Hook grenadeExplosionHook;
void grenadeExplosion(int itemID);
extern subhook::Hook serverPlayerMessageHook;
int serverPlayerMessage(int playerID, char* message);
extern subhook::Hook playerAIHook;
void playerAI(int playerID);
extern subhook::Hook playerDeathTaxHook;
void playerDeathTax(int playerID);

extern subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
void addCollisionRigidBodyOnRigidBody(int aBodyID, int bBodyID,
                                      Vector* aLocalPos, Vector* bLocalPos,
                                      Vector* normal, float, float, float,
                                      float);

extern subhook::Hook createEventMessageHook;
void createEventMessage(int speakerType, char* message, int speakerID,
                        int distance);
extern subhook::Hook createEventUpdatePlayerHook;
void createEventUpdatePlayer(int id);
extern subhook::Hook createEventUpdateVehicleHook;
void createEventUpdateVehicle(int vehicleID, int updateType, int partID,
                              Vector* pos, Vector* normal);
extern subhook::Hook createEventBulletHook;
void createEventBullet(int bulletType, Vector* pos, Vector* vel, int itemID);
extern subhook::Hook createEventBulletHitHook;
void createEventBulletHit(int unk, int hitType, Vector* pos, Vector* normal);

extern subhook::Hook lineIntersectHumanHook;
int lineIntersectHuman(int humanID, Vector* posA, Vector* posB);
};  // namespace Hooks