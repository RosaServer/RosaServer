#pragma once
#include <set>
#include <unordered_map>

#include "structs.h"
#include "subhook.h"

namespace Hooks {
extern sol::protected_function run;

enum EnableKeys {
	ResetGame,
	CreateTraffic,
	TrafficSimulation,
	TrafficCarAI,
	TrafficCarDestination,
	AreaCreateBlock,
	AreaDeleteBlock,
	InterruptSignal,
	Logic,
	ConsoleInput,
	ConsoleAutoComplete,
	LogicRace,
	LogicRound,
	LogicWorld,
	LogicTerminator,
	LogicCoop,
	LogicVersus,
	PlayerActions,
	Physics,
	PhysicsRigidBodies,
	ServerReceive,
	ServerSend,
	PacketBuilding,
	CalculateEarShots,
	SendPacket,
	PhysicsBullets,
	EconomyCarMarket,
	AccountsSave,
	AccountTicketBegin,
	AccountTicketFound,
	AccountTicket,
	SendConnectResponse,
	ItemLink,
	ItemComputerInput,
	HumanDamage,
	HumanCollisionVehicle,
	HumanLimbInverseKinematics,
	GrenadeExplode,
	VehicleDamage,
	PlayerChat,
	PlayerAI,
	PlayerDeathTax,
	AccountDeathTax,
	PlayerGiveWantedLevel,
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
	EventUpdateItemInfo,
	EventUpdatePlayer,
	EventUpdateVehicle,
	EventSound,
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

extern subhook::Hook createTrafficHook;
void createTraffic(int amount);

extern subhook::Hook trafficSimulationHook;
void trafficSimulation();

extern subhook::Hook aiTrafficCarHook;
void aiTrafficCar(int id);

extern subhook::Hook aiTrafficCarDestinationHook;
void aiTrafficCarDestination(int id, int a, int b, int c, int d);

extern subhook::Hook areaCreateBlockHook;
void areaCreateBlock(int zero, int blockX, int blockY, int blockZ,
                     unsigned int flags, short[8]);
extern subhook::Hook areaDeleteBlockHook;
void areaDeleteBlock(int zero, int blockX, int blockY, int blockZ);

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
extern subhook::Hook rigidBodySimulationHook;
void rigidBodySimulation();
extern subhook::Hook serverReceiveHook;
int serverReceive();
extern subhook::Hook serverSendHook;
void serverSend();
extern subhook::Hook packetWriteHook;
int packetWrite(void* source, int elementSize, int elementCount);
extern subhook::Hook calculatePlayerVoiceHook;
void calculatePlayerVoice(int connectionID, int playerID);
extern subhook::Hook sendPacketHook;
int sendPacket(unsigned int address, unsigned short port);
extern subhook::Hook bulletSimulationHook;
void bulletSimulation();

extern subhook::Hook economyCarMarketHook;
void economyCarMarket();
extern subhook::Hook saveAccountsServerHook;
void saveAccountsServer();

extern subhook::Hook createAccountByJoinTicketHook;
int createAccountByJoinTicket(int identifier, unsigned int ticket);
extern subhook::Hook serverSendConnectResponseHook;
void serverSendConnectResponse(unsigned int address, unsigned int port, int unk,
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
extern subhook::Hook humanLimbInverseKinematicsHook;
void humanLimbInverseKinematics(int, int, int, Vector*, RotMatrix*, Vector*,
                                float, float, float, float* /* Quaternion? */,
                                Vector*, Vector*, Vector*, char);
extern subhook::Hook grenadeExplosionHook;
void grenadeExplosion(int itemID);
extern subhook::Hook vehicleApplyDamageHook;
void vehicleApplyDamage(int vehicleID, int damage);
extern subhook::Hook serverPlayerMessageHook;
int serverPlayerMessage(int playerID, char* message);
extern subhook::Hook playerAIHook;
void playerAI(int playerID);
extern subhook::Hook playerDeathTaxHook;
void playerDeathTax(int playerID);
extern subhook::Hook accountDeathTaxHook;
void accountDeathTax(int accountID);
extern subhook::Hook playerGiveWantedLevelHook;
void playerGiveWantedLevel(int playerID, int victimPlayerID, int basePoints);

extern subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
void addCollisionRigidBodyOnRigidBody(int aBodyID, int bBodyID,
                                      Vector* aLocalPos, Vector* bLocalPos,
                                      Vector* normal, float, float, float,
                                      float);

extern subhook::Hook createEventMessageHook;
void createEventMessage(int speakerType, char* message, int speakerID,
                        int distance);
extern subhook::Hook createEventUpdateItemInfoHook;
void createEventUpdateItemInfo(int id);
extern subhook::Hook createEventUpdatePlayerHook;
void createEventUpdatePlayer(int id);
extern subhook::Hook createEventUpdateVehicleHook;
void createEventUpdateVehicle(int vehicleID, int updateType, int partID,
                              Vector* pos, Vector* normal);
extern subhook::Hook createEventSoundHook;
void createEventSound(int soundType, Vector* pos, float volume, float pitch);
extern subhook::Hook createEventBulletHook;
void createEventBullet(int bulletType, Vector* pos, Vector* vel, int itemID);
extern subhook::Hook createEventBulletHitHook;
void createEventBulletHit(int unk, int hitType, Vector* pos, Vector* normal);

extern subhook::Hook lineIntersectHumanHook;
int lineIntersectHuman(int humanID, Vector* posA, Vector* posB, float padding);
extern subhook::Hook lineIntersectLevelHook;
int lineIntersectLevel(Vector* posA, Vector* posB, int unk);

struct Float {
	float value;
};

struct Integer {
	int value;
};

struct UnsignedInteger {
	unsigned int value;
};

extern bool isInBulletSimulation;
};  // namespace Hooks
